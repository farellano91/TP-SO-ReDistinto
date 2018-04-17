#include "cliente.h"

void saludo_inicial_servidor(int sockfd,char* nombre) {
	//Recibo saludo
	void* bufferRecibido = malloc(sizeof(char) * 25);
	char * mensajeSaludoRecibido = malloc(sizeof(char) * 25);
	int numbytes = 0;
	if ((numbytes = recv(sockfd, bufferRecibido, 25, 0)) == -1) {
		perror("recv");
		printf("No se pudo recibir saludo del %s\n",nombre);
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
		perror("recv");
		exit(1);
	}
	printf("Envie mi saludo al %s exitosamente\n",nombre);

	free(bufferRecibido);
	free(mensajeSaludoEnviado);
	free(bufferEnvio);
	free(mensajeSaludoRecibido);

}


int conectar_servidor(char* puerto, char* ip, char * nombre) {
	//busco la ip y puerto
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config del %s\n",nombre);
		exit(1);
	}
	int PORT = config_get_int_value(config, puerto);
	char * IP = malloc(sizeof(char) * 100);
	strcpy(IP, config_get_string_value(config, ip));
	config_destroy(config);

	//Creamos un socket
	int sockfd;
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("ERROR socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(IP);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		printf("No me pude conectar al %s\n",nombre);
		exit(1);
	}

	free(IP);
	return (sockfd);
}

