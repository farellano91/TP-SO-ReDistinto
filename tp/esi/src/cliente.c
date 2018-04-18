#include "cliente.h"

void saludo_inicial_servidor(int sockfd, char* nombre) {
	//Recibo saludo
	void* bufferRecibido = malloc(sizeof(char) * 25);
	char * mensajeSaludoRecibido = malloc(sizeof(char) * 25);
	int numbytes = 0;
	if ((numbytes = recv(sockfd, bufferRecibido, 25, 0)) == -1) {
		printf("No se pudo recibir saludo del %s\n", nombre);

		free(bufferRecibido);
		free(mensajeSaludoRecibido);

		//MUERO
		exit(1);
	}
	memcpy(mensajeSaludoRecibido, bufferRecibido, 25);
	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	//Envio saludo
	void* bufferEnvio = malloc(sizeof(char) * 16);
	char * mensajeSaludoEnviado = malloc(sizeof(char) * 16);
	strcpy(mensajeSaludoEnviado, "Hola soy un ESI");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';
	memcpy(bufferEnvio, mensajeSaludoEnviado, 16);

	if (send(sockfd, bufferEnvio, 16, 0) == -1) {
		printf("No se pudo enviar saludo al %s\n", nombre);

		free(mensajeSaludoEnviado);
		free(bufferEnvio);

		//MUERO
		exit(1);
	}
	printf("Envie mi saludo al %s exitosamente\n", nombre);

	free(bufferRecibido);
	free(mensajeSaludoEnviado);
	free(bufferEnvio);
	free(mensajeSaludoRecibido);

}

int conectar_servidor(int puerto , char* ip, char* nombre) {

	//Creamos un socket
	int sockfd;
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("ERROR socket");
		free_parametros_config();
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(puerto);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(ip);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		printf("No me pude conectar al %s\n", nombre);
		free_parametros_config();
		exit(1);
	}

	return (sockfd);
}

