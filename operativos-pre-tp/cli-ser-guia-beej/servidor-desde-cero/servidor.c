/*
 ** server.c -- Ejemplo de servidor de sockets de flujo
 */
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MYPORT 8082    // Puerto al que conectarán los usuarios

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

void atenderCliente(void *datoC);

typedef struct DatosCliente {
		   char*  ipCliente;
		   int   idSocketCliente;
	} TipoDatosCliente;

pthread_mutex_t mutex;

int main(void) {
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
	my_addr.sin_addr.s_addr = inet_addr("192.168.0.12"); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
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
		TipoDatosCliente *datosCliente;
		datosCliente = malloc(sizeof(TipoDatosCliente));
		if ((datosCliente->idSocketCliente = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			perror("accept");
		}
		datosCliente->ipCliente = inet_ntoa(their_addr.sin_addr);
		printf("Socket cliente %d de IP %s\n", datosCliente->idSocketCliente,
				datosCliente->ipCliente);

		//CREAMOS UN HILO PARA ATENDERLO
		pthread_t punteroHilo;
		pthread_create(&punteroHilo, NULL, (void*) atenderCliente, datosCliente);

	}
	return 0;
}

//Funcion que envia un saludo al cliente y se queda esperando los mensajes del cliente
void atenderCliente(void *datoC) {
	TipoDatosCliente *datosCliente;
	datosCliente = malloc(sizeof(TipoDatosCliente));
	//igualo los punteros
	datosCliente = datoC;
	int idSocket = datosCliente->idSocketCliente;
	char * ipClienteConectado = datosCliente->ipCliente;
	pthread_mutex_lock(&mutex);
	if (send(idSocket, "Hola cliente, todo bien?\n", 25, 0) == -1) {
		printf("No se pudo enviar saludo al cliente");
	}
	while (1) {
		//Recibimos el mensaje
		char* buffer = malloc(sizeof(char*));
		int bytesRecibidos = recv(idSocket, buffer, 10, 0);//10->4: como maximo 4 bytes
		if (bytesRecibidos <= 0) {
			printf("se fue el cliente de IP %s\n",ipClienteConectado);
			break;
		} else {
			if (strcmp("exit", buffer) != 0) {
				printf("Llegaron %d bytes con mesaje %s del cliente IP%s\n", bytesRecibidos,
						buffer,ipClienteConectado);
				free(buffer);
			} else {
				printf("se fue el cliente de IP %s\n",ipClienteConectado);
				break;
			}

		}
	}
	pthread_mutex_unlock(&mutex);
}
