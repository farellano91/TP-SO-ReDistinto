#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

int recibir_saludo(int fdCliente) {
	int resultado = 0;
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(fdCliente, &longitud, sizeof(int32_t), 0)) == -1) {
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fdCliente, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		//MUERO
		exit(1);
	}
	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	if (strstr(mensajeSaludoRecibido, "ESI") != NULL) {
		resultado = 1;
	}
	if (strstr(mensajeSaludoRecibido, "PLA") != NULL) {
		resultado = 2;
	}
	if (strstr(mensajeSaludoRecibido, "INS") != NULL) {
		resultado = 3;
	}

	free(mensajeSaludoRecibido);
	return resultado;
}

/*PROTOCOLO para envio saludo al ESI
 * ESI <-> PLANIFICADOR
 * [INT + CHAR*]
 * INT: len del mensaje saludos
 * CHAR*: mensaje saludo
 * */

void enviar_saludo(int fdCliente) {

	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el COORDINADOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(fdCliente, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);


}

void recibo_lineas(int fd_esi) {
	int longitud = 0;
	int numbytes = 0;

	while (1) {
		sleep(retardo);
		void* bufferMensaje = malloc(sizeof(int32_t));
		if ((numbytes = recv(fd_esi, bufferMensaje, sizeof(int32_t), 0))
				<= 0) {
			if (numbytes == 0) {
				//si el cliente se fue
				printf("Se fue: ESI de fd:%d, chau gato!!!\n", fd_esi);
				free(bufferMensaje);
				break;
			}

		} else {
			memcpy(&longitud, bufferMensaje, sizeof(int32_t));
			char * linea = malloc(sizeof(char) * longitud);
			void* buffer = malloc(sizeof(char) * longitud);

			if ((numbytes = recv(fd_esi, buffer, sizeof(char) * longitud, 0))
					<= 0) {
				if (numbytes == 0) {
					//si el cliente se fue
					printf("Se fue: ESI de fd:%d, chau gato!!!\n", fd_esi);
					free(linea);
					free(bufferMensaje);
					free(buffer);
					break;
				}
			} else {

				memcpy(linea, buffer, sizeof(char) * longitud);
				printf("Recibi linea: %s\n", linea);

				//recibo el id del esi
				int32_t id_esi = 0;
				if ((numbytes = recv(fd_esi, &id_esi, sizeof(int32_t), 0))
						<= 0) {
					if (numbytes == 0) {
						//si el cliente se fue
						printf("Se fue: ESI de fd:%d, chau gato!!!\n", fd_esi);
						free(linea);
						free(bufferMensaje);
						free(buffer);
						break;
					}
				}else{
					printf("ESI id:%d\n", id_esi);
					//Consulto si tengo un planificador conectado y si es GET/STORE lo que recibi
					//-> envio al planificador la linea
					if (fd_planificador != -1) {

						log_info(logger,linea);

						if ((strstr(linea, "STORE") != NULL) || (strstr(linea, "GET") != NULL)) {

							t_InfoCoordinador infoCoordinador = {.id = 0 , .clave ="" , .id_esi= 0};

							if (strstr(linea, "GET") != NULL) {
								infoCoordinador.id = 1;
							} else {
								infoCoordinador.id = 2;
							}
							strcpy(infoCoordinador.clave, linea);
							infoCoordinador.id_esi = id_esi;
							if (send(fd_planificador, &infoCoordinador,
									sizeof(t_InfoCoordinador), 0) == -1) {
								printf("No se pudo enviar la info\n");
								exit(1);
							}
							printf("Se envio la INFO al PLANIFICADOR correctamente\n");

							//TODO:Aca recibo la respuesta del planificador

						} else {
							//TODO: si no es un get ni store, entonces es un set:
							//1.- aplicar algoritmo de distribucion para decidir q instancia va
							//(usando una cola de instancias o cola de struct Instancia ;))
							//2.- enviar la peticion a la instancia elegida
							//3.- recibir una respuesta de la instancia

						}

						//envio resultado al esi (por ahora todo es OK)
						int32_t resultado_coordinador = 2;  //1:falle , 2:ok , 3: ok pero te bloqueaste
						if (send(fd_esi, &resultado_coordinador,sizeof(int32_t), 0) == -1) {
							printf("No se pudo enviar resultado al ESI\n");
							exit(1);
						}
						printf("Envie resultado al ESI de ID:%d!!!\n",id_esi);
					}
				}






			}
			free(linea);
			free(bufferMensaje);
			free(buffer);
		}
	}

}

/*PROTOCOLO:
 * tipo_cliente: 1 -> ESI
 * tipo_cliente: 2 -> PLANIFICADOR
 * tipo_cliente: 3 -> INSTANCIA
 * */
void atender_cliente(void* idSocketCliente) {

	//Saludo a todos los q se conectaron
	int fdCliente = ((int *) idSocketCliente)[0];

	enviar_saludo(fdCliente);
	int tipo_cliente = recibir_saludo(fdCliente);

	switch (tipo_cliente) {

	case 1:
		//ESI
		recibo_lineas(fdCliente);
		break;
	case 2:
		//PLANIFICADOR (me guardo el fd)
		fd_planificador = fdCliente;
		while (1) {
			//- No corto la comunicacion con el planificador -
		}
		break;
	case 3:
		//INSTANCIA
		//1:Envio datos de entrada
		envio_datos_entrada(fdCliente);
		//2:Recibo datos para crear la instancia
		t_Instancia* instancia_nueva = creo_instancia(fdCliente);
		//3:Encolo la INSTANCIA
		agrego_instancia_lista(list_instancias,instancia_nueva);
		break;

	}
	//FIN
	close(fdCliente);
	free((int *) idSocketCliente);

}

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		free_parametros_config();
		log_destroy(logger);
		exit(dummy);

	}
}
void levantar_servidor_coordinador() {
	fd_planificador = -1;

	//creo mi lista de instancia
	list_instancias = create_list_instancias();

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error al abrir el socket de escucha\n");
		free_parametros_config();
		exit(1);
	}
	printf("Se creo el socket correctamente\n");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		printf("Address already in use\n");
		free_parametros_config();
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(puerto_escucha_conexion);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(MYIP); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf("Fallo el bind\n");
		free_parametros_config();
		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {
		free_parametros_config();
		printf("Fallo el listen\n");
		exit(1);
	}
	printf("Socket escuchando!!!\n");

	//-------
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	//--------
	sin_size = sizeof(struct sockaddr_in);

	//4° acepta y atiende a los ESI's, INSTANCIAS's que se conecte
	while (1) {
		int *idSocketCliente = (int *) malloc(sizeof(int32_t));
		idSocketCliente[0] = -1; //TODO: que pasa con esta variable con varios hilos ?
		if ((idSocketCliente[0] = accept(sockfd,
				(struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("Error al usar accept");
		}

		//CREAMOS UN HILO PARA ATENDER A CUALQUIER CLIENTE
		pthread_t punteroHilo;
		pthread_create(&punteroHilo, NULL, (void*) atender_cliente,
				idSocketCliente);

		//espero a q termine el hilo (para evitar quilombo por ahora)
		//pthread_join(punteroHilo, NULL);

	}
	free_parametros_config();
	log_destroy(logger);
	close(sockfd);
}

