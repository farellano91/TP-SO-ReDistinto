/*
 * servidor.c
 *
 *  Created on: 25/3/2018
 *      Author: utnso
 */
#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

//Funcion que envia un saludo al cliente y se queda esperando los mensajes del cliente
void atenderCliente(TipoDatosCliente *datoC) {
	
	//por ahora sin semaforo
	//pthread_mutex_lock(&mutex);

	char * saludo = malloc(sizeof(char)*24 + 1);
	strcpy(saludo,"Hola cliente, todo bien?");
	saludo[strlen(saludo)]= '\0';
	
	//Quise enviar todo el mensaje con el \0 para q solo lo muestre del otro lado
	if (send(datoC->idSocketCliente, saludo, 25, 0) == -1) {
		printf("No se pudo enviar saludo al cliente");
	}
	
	while (1) {
		//Vacia los datos de salida estandar
		fflush(stdout);
		//Recibimos el mensaje (Se establece un protocolo donde el primer bytes es el numero de caracteres y el resto es el mensaje)
		void* buffer = malloc(sizeof(int32_t) + sizeof(char) * 20);
		//recibo todo y lo meto en 24bytes porque desconosco cuanto llega posta
		int bytesRecibidos = recv(datoC->idSocketCliente, buffer, sizeof(int32_t) + sizeof(char) * 20, 0);//10->4: como maximo 4 bytes

		//Deserializo
		int tamanioMensaje = 0;
		memcpy(&tamanioMensaje,buffer,sizeof(int32_t));

		char* mensaje = malloc(sizeof(char) * (tamanioMensaje + 1));
		
		memcpy(mensaje, (buffer + sizeof(int32_t)), tamanioMensaje*sizeof(char) );
		
		//se agrega \0 para poder imprimir sin problema
		mensaje[tamanioMensaje] = '\0';
		
		if (bytesRecibidos <= 0) {
			printf("se fue el cliente de IP %s\n",datoC->ipCliente);
			
			free(buffer);
			free(mensaje);
			
			break;
		} else {
			if (strcmp(mensaje,"exit") != 0) {
				printf("Llego el mensaje %s de longitud %d del cliente IP %s\n",
						mensaje,tamanioMensaje,datoC->ipCliente);
				
				free(buffer);
				free(mensaje);

			} else {
				printf("se fue el cliente de IP %s\n",datoC->ipCliente);
				
				free(buffer);
				free(mensaje);

				break;
			}

		}
	}
	free(datoC->ipCliente);
	free(datoC);
	free(saludo);
	//por ahora sin semaforo
	//pthread_mutex_unlock(&mutex);
}

void levantarServidor() {
	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		printf("Error al abrir el socket de escucha\n");
		exit(1);
	}
	printf("Se creo el socket %d\n", sockfd);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
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

	//4° Accept idSocketCliente: numero de socket del cliente que se acaba de conectar
	while (1) {
		
		TipoDatosCliente * datosCliente = malloc(sizeof(TipoDatosCliente));
		datosCliente->ipCliente = malloc(sizeof(char) * 30);

		puts("Ingrese 'si' para recibir una nueva conexion o 'no' para terminar ");
		//scanf solo inserta 2 y el 3ro lo deja para el \0
		char * entrada = malloc(sizeof(char) * 2 + 1);
		//espero de entrada solo SI  o NO 
		scanf("%s",entrada);
		if(strcmp(entrada,"no") == 0){

			free(entrada);
			free(datosCliente->ipCliente);
			free(datosCliente);
			break;
		}
		free(entrada);

		if ((datosCliente->idSocketCliente = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			perror("accept");
		}
		
		strcpy(datosCliente->ipCliente,inet_ntoa(their_addr.sin_addr));
		
		printf("Socket cliente %d de IP %s\n", datosCliente->idSocketCliente,
				datosCliente->ipCliente);

		//CREAMOS UN HILO PARA ATENDERLO
		pthread_t punteroHilo;
		pthread_create(&punteroHilo, NULL, (void*) atenderCliente, datosCliente);
		//espero a q termine el hilo
		pthread_join(punteroHilo,NULL);

	}
}


