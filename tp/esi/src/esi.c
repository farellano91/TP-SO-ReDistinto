#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_esi.h"
#include "archivo.h"
#include "cliente.h"

void exit_gracefully(int return_nr) {
	exit(return_nr);
}

int main(int argc, char** argv) {

	get_parametros_config();

	//Conecta como cliente al coordinador
	int fd_coordinador = conectar_servidor(PUERTO_CONFIG_COORDINADOR,
			IP_CONFIG_COORDINADOR, "COORDINADOR");
	saludo_inicial_servidor(fd_coordinador, "COORDINADOR");

	//Conecta como cliente al planificador
	int fd_planificador = conectar_servidor(PUERTO_CONFIG_PLANIFICADOR,
			IP_CONFIG_PLANIFICADOR, "PLANIFICADOR");
	saludo_inicial_servidor(fd_planificador, "PLANIFICADOR");

	free_parametros_config();

	//2.- lee el archivo linea por linea
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen(argv[1], "r");
	//fp = fopen("script.esi", "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
		close(fd_coordinador);
		exit(EXIT_FAILURE);
	}

	int numbytes = 0;
	int32_t permiso = 0; //1: OK
	int32_t resultado_coordinador = 0; //1:falle , 2:ok , 3: ok pero te bloqueaste

	int32_t longitud_valor = 0;
	int32_t longitud_clave = 0;
	printf("Espero permiso para empezar...\n");
	while ((read = getline(&line, &len, fp)) != -1) {

		//recv de permiso por parte del planificador (sabemos que los permisos son solo int32_t)
		if ((numbytes = recv(fd_planificador, &permiso, sizeof(int32_t), 0))
				<= 0) {

			if (numbytes == 0) {
				// conexión cerrada
				printf("Se desconecto el planificador\n");
			} else {
				perror("ERROR: al recibir respuesta del planificador");
			}
			close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
			close(fd_coordinador);
			fclose(fp);
			if (line)
				free(line);
			exit(1); //muero
		} else {
			if (permiso) { //en teoria siempre si recibo algo es 1, sino seguiria esperando arriba

				//Envio ESI + DATOS DE LA SENTENCIA QUE LEO al coordinador
				t_esi_operacion parsed = parse(line);
				int32_t operacion = 0; //1:GET 2:SET 3:STORE
				if (parsed.valido) {
					switch (parsed.keyword) {
					case GET:

						operacion = 1;
						longitud_clave = strlen(parsed.argumentos.GET.clave)
								+ 1;

						//envio al coordinador ID_ESI+OPERACION+CLAVE
						void* bufferEnvio_get = malloc(
								sizeof(int32_t) * 3 + longitud_clave);
						memcpy(bufferEnvio_get, &ID_ESI_OBTENIDO,
								sizeof(int32_t));
						memcpy(bufferEnvio_get + sizeof(int32_t), &operacion,
								sizeof(int32_t));
						memcpy(bufferEnvio_get + sizeof(int32_t) * 2,
								&longitud_clave, sizeof(int32_t));
						memcpy(bufferEnvio_get + sizeof(int32_t) * 3,
								parsed.argumentos.GET.clave, longitud_clave);
						if (send(fd_coordinador, bufferEnvio_get,
								(sizeof(int32_t) * 3) + longitud_clave, 0)
								== -1) {
							perror("send");
							exit(1);
						}
						printf("Envie correctamente: GET clave: <%s>\n",
								parsed.argumentos.GET.clave);
						free(bufferEnvio_get);
						break;
					case SET:

						operacion = 2;
						longitud_valor = strlen(
								parsed.argumentos.SET.valor) + 1;
						longitud_clave = strlen(
								parsed.argumentos.SET.clave) + 1;

						//envio al coordinador ID_ESI + OPERACION + CLAVE + VALOR
						void* bufferEnvio_set = malloc(
								sizeof(int32_t) * 4 + longitud_valor
										+ longitud_clave);

						memcpy(bufferEnvio_set, &ID_ESI_OBTENIDO,
								sizeof(int32_t));
						memcpy(bufferEnvio_set + sizeof(int32_t), &operacion,
								sizeof(int32_t));
						memcpy(bufferEnvio_set + sizeof(int32_t) * 2,
								&longitud_clave, sizeof(int32_t));
						memcpy(bufferEnvio_set + sizeof(int32_t) * 3, parsed.argumentos.SET.clave,
								longitud_clave);
						memcpy(
								bufferEnvio_set + sizeof(int32_t) * 3
										+ longitud_clave, &longitud_valor,
								sizeof(int32_t));
						memcpy(
								bufferEnvio_set + sizeof(int32_t) * 4
										+ longitud_clave, parsed.argumentos.SET.valor,
								longitud_valor);

						if (send(fd_coordinador, bufferEnvio_set,
								sizeof(int32_t) * 4 + longitud_valor
										+ longitud_clave, 0) == -1) {
							perror("send");
							exit(1);
						}
						printf(
								"Envie correctamente: SET clave: <%s> valor: <%s>\n",
								parsed.argumentos.SET.clave,
								parsed.argumentos.SET.valor);
						free(bufferEnvio_set);
						break;
					case STORE:

						operacion = 3;
						longitud_clave = strlen(parsed.argumentos.STORE.clave)
								+ 1;

						//envio al coordinador ID_ESI + OPERACION+CLAVE
						void* bufferEnvio_store = malloc(
								sizeof(int32_t) * 3 + longitud_clave);

						memcpy(bufferEnvio_store, &ID_ESI_OBTENIDO,
								sizeof(int32_t));
						memcpy(bufferEnvio_store + sizeof(int32_t), &operacion,
								sizeof(int32_t));
						memcpy(bufferEnvio_store + sizeof(int32_t) * 2,
								&longitud_clave, sizeof(int32_t));
						memcpy(bufferEnvio_store + sizeof(int32_t) * 3,
								parsed.argumentos.STORE.clave, longitud_clave);

						if (send(fd_coordinador, bufferEnvio_store,
								sizeof(int32_t) * 3 + longitud_clave, 0)
								== -1) {
							perror("recv");
							exit(1);
						}
						printf("Envie correctamente: STORE clave: <%s>\n",
								parsed.argumentos.STORE.clave);
						free(bufferEnvio_store);

						break;
					default:
						fprintf(stderr, "No pude interpretar <%s>\n", line);
						close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
						close(fd_coordinador);
						exit(EXIT_FAILURE);
					}

				}


				//recv resultado de la sentencia q le mande al coordinador!!!!
				if ((numbytes = recv(fd_coordinador, &resultado_coordinador,
						sizeof(int32_t), 0)) <= 0) {
					if (numbytes == 0) {
						// conexión cerrada
						printf("Se desconecto el coordinador\n");
					} else {
						perror("ERROR: al recibir respuesta del coordinador");
					}
					close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
					close(fd_coordinador);
					exit(1); //muero
				} else {

					t_respuesta_para_planificador respuesta_planificador = {
							.id_tipo_respuesta = 2, .id_esi = ID_ESI_OBTENIDO,
							.mensaje = "", .clave=""};
					switch (resultado_coordinador) {
					case 1: //recibi respuesta q coordinador no lo pudo hacer
						strcpy(respuesta_planificador.mensaje, "MAL");
						respuesta_planificador.mensaje[strlen(
								respuesta_planificador.mensaje)] = '\0';
						break;
					case 2: //recibi respuesta q coordinador lo hizo bien
						strcpy(respuesta_planificador.mensaje, "OK");
						respuesta_planificador.mensaje[strlen(
								respuesta_planificador.mensaje)] = '\0';
						break;
					case 3: //recibi respuesta q coordinador trato pero esta bloqueado
						strcpy(respuesta_planificador.mensaje, "ME BLOQUEARON");
						respuesta_planificador.mensaje[strlen(
								respuesta_planificador.mensaje)] = '\0';

						//si me bloquearon es por un GET, entonces mando mi clave
						strcpy(respuesta_planificador.clave,parsed.argumentos.GET.clave);
						respuesta_planificador.clave[strlen(
								respuesta_planificador.clave)] = '\0';
						break;
					}

					//send resultado al planificador
					if (send(fd_planificador, &respuesta_planificador,
							sizeof(t_respuesta_para_planificador), 0) == -1) {
						printf("No se pudo enviar respuesta al planificador\n");
						exit(1);
					}
					printf("Respuesta enviado al planificador correctamente\n");

				}
				destruir_operacion(parsed);
			}

		}

	}

	//recv de permiso por parte del planificador (sabemos que los permisos son solo int32_t)
	if ((numbytes = recv(fd_planificador, &permiso, sizeof(int32_t), 0))
			<= 0) {

		if (numbytes == 0) {
			// conexión cerrada
			printf("Se desconecto el planificador\n");
		} else {
			perror("ERROR: al recibir respuesta del planificador");
		}
		close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
		close(fd_coordinador);
		fclose(fp);
		if (line)
			free(line);
		exit(1); //muero
	}else{
		if(permiso){
			//Send resultado al planificador q ya no tengo mas lineas para leer
			t_respuesta_para_planificador respuesta_planificador = {
					.id_tipo_respuesta = 3, .id_esi = ID_ESI_OBTENIDO, .mensaje = ""};
			strcpy(respuesta_planificador.mensaje, "TERMINE DE LEER TODO CAPO");
			respuesta_planificador.mensaje[strlen(respuesta_planificador.mensaje)] =
					'\0';

			if (send(fd_planificador, &respuesta_planificador,
					sizeof(t_respuesta_para_planificador), 0) == -1) {
				printf("No se pudo enviar respuesta al planificador\n");
				exit(1);
			}
		}

		printf("TERMINE!!\n");
	}



	fclose(fp);
	if (line)
		free(line);

	close(fd_planificador);
	close(fd_coordinador);
	return EXIT_SUCCESS;
}
