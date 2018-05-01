
#include "cliente.h"

void saludo_inicial_coordinador(int sockfd){
	//Recibo saludo
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(sockfd, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del saludo del coordinador\n");
		free_algo_punt_nom();
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(sockfd, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir saludo\n");
		free_algo_punt_nom();
		free(mensajeSaludoRecibido);
		//MUERO
		exit(1);
	}

	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	free(mensajeSaludoRecibido);
	//Envio saludo
	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy la INSTANCIA");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(sockfd, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		printf("No se pudo enviar saludo\n");
		free_algo_punt_nom();
		free(bufferEnvio);
		free(mensajeSaludoEnviado);
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);



}



int conectar_coodinador(){

	//Creamos un socket
	int sockfd;
    struct sockaddr_in their_addr; // información de la dirección de destino

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        free_parametros_config();
        exit(1);
    }

    their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
    their_addr.sin_port = htons(PUERTO_CONFIG_COORDINADOR);  // short, Ordenación de bytes de la red
    their_addr.sin_addr.s_addr = inet_addr(IP_CONFIG_COORDINADOR);//toma la ip directo

    memset( &(their_addr.sin_zero) , 0 , 8);  // poner a cero el resto de la estructura

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                                          sizeof(struct sockaddr)) == -1) {
        perror("No se pudo conectar");
        free_parametros_config();
        exit(1);
    }

    free(IP_CONFIG_COORDINADOR); //por ahora solo libero la ip (que es lo que ya use)
    return(sockfd);
}
