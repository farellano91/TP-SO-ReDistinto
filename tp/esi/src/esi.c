
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
	int fd_coordinador = conectar_servidor(puerto_config_coordinador,ip_config_coordinador,"COORDINADOR");
	saludo_inicial_servidor(fd_coordinador,"COORDINADOR");

	//Conecta como cliente al planificador
	int fd_planificador = conectar_servidor(puerto_config_planificador,ip_config_planificador,"PLANIFICADOR");
	saludo_inicial_servidor(fd_planificador,"PLANIFICADOR");

	free_parametros_config();


//2.- lee el archivo linea por linea
	if (argc != 2) {
		puts("Parametros no correctos..bye bye!");
		exit_gracefully(-1);
	}
	FILE* file = txt_open_file(argv[1], "r");
	if (file == NULL) {
		txt_close_file(file);
		puts("Error al abrir el archivo ");
		exit_gracefully(-1);
	}
	char* line = malloc(sizeof(char) * 500);
	int numbytes = 0;
	int32_t permiso = 0; //1: OK
	int32_t resultado_coordinador = 0; //1:falle , 2:ok , 3: ok pero te bloqueaste
	while (fgets(line, 500, file)) {
		//TODO:parseo la linea

		//recv de permiso por parte del planificador (sabemos que los permisos son solo int32_t)
		if ((numbytes = recv(fd_planificador, &permiso, sizeof(int32_t), 0)) <= 0) {

			if (numbytes == 0) {
			// conexión cerrada
				printf("Se desconecto el planificador\n");
			} else {
				perror("ERROR: al recibir respuesta del planificador");
			}
			close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
			close(fd_coordinador);
			exit(1); //muero
		}else{
			if(permiso){ //en teoria siempre si recibo algo es 1, sino seguiria esperando arriba
				//envio linea y mi ID para q el coordinador lo sepa
				int longitud = strlen(line) + 1;
				line[strlen(line)] = '\0';
				void* bufferEnvio = malloc(sizeof(int32_t) + longitud + sizeof(int32_t));
				memcpy(bufferEnvio, &longitud, sizeof(int32_t));
				memcpy(bufferEnvio + sizeof(int32_t), line, longitud);
				memcpy(bufferEnvio + sizeof(int32_t)+longitud, &id_esi_obtenido, sizeof(int32_t));

				if (send(fd_coordinador, bufferEnvio, sizeof(int32_t) + longitud + sizeof(int32_t), 0) == -1) {
					perror("recv");
					exit(1);
				}
				printf("Envie linea: %s\n", line);
				free(bufferEnvio);
			}
		}

		//recv resultado de la sentencia q le mande al coordinador!!!!
		if ((numbytes = recv(fd_coordinador, &resultado_coordinador, sizeof(int32_t), 0)) <= 0) {
			if (numbytes == 0) {
				// conexión cerrada
				printf("Se desconecto el coordinador\n");
			} else {
				perror("ERROR: al recibir respuesta del planificador");
			}
			close(fd_planificador); // si ya no conversare mas con el cliente, lo cierro
			close(fd_coordinador);
			exit(1); //muero
		}else{

			t_respuesta_para_planificador respuesta_planificador = {.id_tipo_respuesta = 2, .id_esi = id_esi_obtenido,
								.mensaje = "", .instruccion = "" };
			switch (resultado_coordinador) {
			case 1://recibi respuesta q coordinador no lo pudo hacer
				strcpy(respuesta_planificador.mensaje,"MAL");
				respuesta_planificador.mensaje[strlen(respuesta_planificador.mensaje)]='\0';
				break;
			case 2://recibi respuesta q coordinador lo hizo bien
				strcpy(respuesta_planificador.mensaje,"OK");
				respuesta_planificador.mensaje[strlen(respuesta_planificador.mensaje)]='\0';
				break;
			case 3://recibi respuesta q coordinador trato pero esta bloqueado
				strcpy(respuesta_planificador.mensaje,"ME BLOQUEARON");
				respuesta_planificador.mensaje[strlen(respuesta_planificador.mensaje)]='\0';
				break;
			}

			//send resultado al planificador
			if (send(fd_planificador, &respuesta_planificador,sizeof(t_respuesta_para_planificador), 0) == -1) {
				printf("No se pudo enviar respuesta al planificador\n");
				exit(1);
			}
			printf("Respuesta enviado correctamente\n");

		}



	}

	//Send resultado al planificador q ya no tengo mas lineas para leer
	t_respuesta_para_planificador respuesta_planificador = {.id_tipo_respuesta = 3, .id_esi = id_esi_obtenido,
									.mensaje = "", .instruccion = "" };
	strcpy(respuesta_planificador.mensaje,"TERMINE DE LEER TODO CAPO");
					respuesta_planificador.mensaje[strlen(respuesta_planificador.mensaje)]='\0';
	if (send(fd_planificador, &respuesta_planificador,sizeof(t_respuesta_para_planificador), 0) == -1) {
		printf("No se pudo enviar respuesta al planificador\n");
		exit(1);
	}
	printf("Respuesta de que termine enviado correctamente al planificador\n");

	free(line);
	txt_close_file(file);
	close(fd_coordinador);
	return EXIT_SUCCESS;
}
