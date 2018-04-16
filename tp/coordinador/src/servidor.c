#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

int recibir_saludo(int fdCliente) {
	int resultado = 0;
	void* bufferRecibido = malloc(sizeof(char) * 29);
	char * mensajeSaludoRecibido = malloc(sizeof(char) * 29);
	int numbytes = 0;
	if ((numbytes = recv(fdCliente, bufferRecibido, 29, 0)) <= 0) {
		if (numbytes == 0) {
			//si el cliente se fue
			printf("Se fue: socket %d, chau gato!!!\n", fdCliente);
			close(fdCliente); // si ya no conversare mas con el cliente, lo cierro

		}

	} else {
		memcpy(mensajeSaludoRecibido, bufferRecibido, 29);
		printf("Saludo recibido: %s\n", mensajeSaludoRecibido);
	}

	if (strstr(mensajeSaludoRecibido, "ESI") != NULL) {
		resultado = 1;
	}
	free(bufferRecibido);
	free(mensajeSaludoRecibido);
	return resultado;
}

void enviar_saludo(int fdCliente) {

	void* bufferEnvio = malloc(sizeof(char) * 25);
	char * mensajeSaludoEnviado = malloc(sizeof(char) * 25);
	strcpy(mensajeSaludoEnviado, "Hola, soy el COORDINADOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';
	memcpy(bufferEnvio, mensajeSaludoEnviado, 25);
	if (send(fdCliente, bufferEnvio, sizeof(char) * 25, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);
}

void recibo_lineas(int fdCliente) {
	int longitud = 0;
	int numbytes = 0;

	//busco la ip y puerto
	t_config* config = config_create("config.cfg");
	if (!config) {
		perror("No encuentro el archivo config");
		exit(1);
	}
	int retardo = config_get_int_value(config, "RETARDO");

	while (1) {
		sleep(retardo);
		void* bufferMensaje = malloc(sizeof(int32_t));
		if ((numbytes = recv(fdCliente, bufferMensaje, sizeof(int32_t), 0))
				<= 0) {
			if (numbytes == 0) {
				//si el cliente se fue
				printf("Se fue: socket %d, chau gato!!!\n", fdCliente);
				free(bufferMensaje);
				break;
			}

		} else {
			memcpy(&longitud, bufferMensaje, sizeof(int32_t));
			char * linea = malloc(sizeof(char) * longitud);
			void* buffer = malloc(sizeof(char) * longitud);

			if ((numbytes = recv(fdCliente, buffer, sizeof(char) * longitud, 0))
					<= 0) {
				if (numbytes == 0) {
					//si el cliente se fue
					printf("Se fue: socket %d, chau gato!!!\n", fdCliente);
					free(linea);
					free(bufferMensaje);
					free(buffer);
					break;
				}
			} else {
				memcpy(linea, buffer, sizeof(char) * longitud);
				printf("Recibi linea: %s\n", linea);
			}
			free(linea);
			free(bufferMensaje);
			free(buffer);
		}
	}
	config_destroy(config);

}

/*PROTOCOLO:
 * tipo_cliente: 1 -> ESI
 * tipo_cliente:2 -> PLANIFICADOR
 * */
void atender_cliente(void* idSocketCliente) {

	int fdCliente = ((int *) idSocketCliente)[0];

	enviar_saludo(fdCliente);
	int tipo_cliente = recibir_saludo(fdCliente);

	//TODO: case para saber q tipo de cliente es y seguir el flujo
	if (tipo_cliente == 1) {
		recibo_lineas(fdCliente);
	}

	//FIN
	close(fdCliente);
	free((int *) idSocketCliente);

}

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		exit(dummy);

	}
}
void levantar_servidor_coordinador() {
	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	//busco la ip y puerto
	t_config* config = config_create("config.cfg");
	if (!config) {
		perror("No encuentro el archivo config");
		exit(1);
	}
	int MYPORT = config_get_int_value(config, "PUERTO_ESCUCHA_CONEXION");
	config_destroy(config);

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		printf("Error al abrir el socket de escucha\n");
		exit(1);
	}
	printf("Se creo el socket %d\n", sockfd);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("address already in use");
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(MYPORT);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(MYIP); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf("Fallo el bind\n");
		perror("bind");
		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
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
	close(sockfd);
}

