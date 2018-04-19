#include "cliente.h"

//PD:Si ejecuto esto la ip del coordinador ya lo libere ;)
void saludo_inicial_coordinador(int sockfd) {

	//Recibo saludo
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(sockfd, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del saludo\n");
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(sockfd, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir saludo\n");
		//MUERO
		exit(1);
	}

	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);



	//Envio saludo
	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el PLANIFICADOREITOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(sockfd, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);

	free(mensajeSaludoRecibido);

}

int conectar_coodinador() {

	//Creamos un socket
	int sockfd;
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		free(ip_config_coordinador);
		pthread_exit(NULL);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(puerto_config_coordinador);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(ip_config_coordinador);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("No se pudo conectar al coordinador");
		free(ip_config_coordinador);
		pthread_exit(NULL);
	}

	free(ip_config_coordinador);
	return (sockfd);
}

void recibirInfoCoordinador() {

	//SALUDOS DE CONEXION CON EL COORDINADOR
	int fdCoordinador = conectar_coodinador();
	saludo_inicial_coordinador(fdCoordinador);

	t_InfoCoordinador infoCoordinador;
	int numbytes = 0;

	while (1) {
		infoCoordinador.id = 0;
		strcpy(infoCoordinador.clave ,"");

		if ((numbytes = recv(fdCoordinador, &infoCoordinador,
				sizeof(t_InfoCoordinador), 0)) <= 0) {
			if(numbytes < 0 ){
				puts("Error al recibir notificacion del coordinador");
			}else{
				puts("Se fue el coordinador");
			}
			break;
		} else {
			switch (infoCoordinador.id) {
			case 1:
				puts("Recibi un GET!!!!!!!!!!!!!");
				//TODO:CODIGO PARA HACER SI RECIBO GET y mando respuesta
				break;
			case 2:
				puts("Recibi un STORE!!!!!!!!!!!");
				//TODO:CODIGO PARA HACER SI RECIBO STORE y mando respuesta
				break;

			}
		}

	}
}
