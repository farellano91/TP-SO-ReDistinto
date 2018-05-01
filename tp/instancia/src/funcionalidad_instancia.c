
#include "funcionalidad_instancia.h"

void get_parametros_config(){
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");

		//MUERO
		exit(1);
	}

	PUERTO_CONFIG_COORDINADOR = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	INTERVALO_DUMP = config_get_int_value(config,"INTERVALO_DUMP");

	IP_CONFIG_COORDINADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_COORDINADOR,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	ALGORITMO_REEMPLAZO = malloc(sizeof(char) * 100);
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(config, "ALGORITMO_REEMPLAZO"));

	PUNTO_MONTAJE = malloc(sizeof(char) * 100);
	strcpy(PUNTO_MONTAJE,config_get_string_value(config, "PUNTO_MONTAJE"));


	NOMBRE_INSTANCIA = malloc(sizeof(char) * 100);
	strcpy(NOMBRE_INSTANCIA,config_get_string_value(config, "NOMBRE_INSTANCIA"));

	config_destroy(config);
}


void free_parametros_config(){

	free(IP_CONFIG_COORDINADOR);
	free(ALGORITMO_REEMPLAZO);
	free(PUNTO_MONTAJE);
	free(NOMBRE_INSTANCIA);
}

void envio_resultado_al_coordinador(sockfd,resultado){

	if(send(sockfd, &resultado, sizeof(int32_t), 0) == -1) {
		printf("No se puede enviar el resultado al coordinador\n");
		exit(1);
	}
	printf("Envie mi resultado correctamente\n");
}

//recibe la linea, la procesa ... y retorna un valor
int recibo_sentencia(int fd_coordinador){
	int32_t long_clave = 0;
	int32_t long_valor = 0;
	int32_t numbytes = 0;

	if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir le tamaño de la clave\n");
		exit(1);
	}

	char* clave_recibida = malloc(sizeof(char)*long_clave);
	if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) == -1) {
		printf("No se pudo recibir la clave\n");
		exit(1);
	}

	if ((numbytes = recv(fd_coordinador, &long_valor, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir le tamaño del valor\n");
		exit(1);
	}

	char* valor_recibido = malloc(sizeof(char)*long_valor);
	if ((numbytes = recv(fd_coordinador, valor_recibido, long_clave, 0)) == -1) {
		printf("No se pudo recibir el valor\n");
		exit(1);
	}
	printf("Recibi para hacer SET clave: %s valor: %s\n",clave_recibida,valor_recibido);

	/*PROCESO.....*/
	//proceso_operacion(clave_recibida,valor_recibido);

	free(clave_recibida);
	free(valor_recibido);

	//1: falla 2: OK
	return 2;
}


void recibo_datos_entrada(int fd_coordinador){
	void* buffer = malloc(sizeof(int32_t)*2);
	int numbytes = 0;
	if ((numbytes = recv(fd_coordinador, &TAMANIO_ENTRADA, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño de la entrada\n");
		exit(1);
	}
	if ((numbytes = recv(fd_coordinador, &CANT_ENTRADA, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la cantidad de entradas\n");
		exit(1);
	}
	printf("Recibi los datos de las entradas correctamente\n");
	free(buffer);
}

//esto para q el coordinador puedo crear su t_instancia
void envio_datos(int fd_coordinador){
	//revisar
	int32_t longitud_mensaje = strlen(NOMBRE_INSTANCIA) + 1;
	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),NOMBRE_INSTANCIA,longitud_mensaje);

	if (send(fd_coordinador, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		printf("No pude enviar mis datos al coordinador\n");
		exit(1);
	}
	printf("Envie mi nombre correctamente\n");
	free(bufferEnvio);
}


