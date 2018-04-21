
#include "funcionalidad_instancia.h"

void get_parametros_config(){
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");

		//MUERO
		exit(1);
	}

	puerto_config_coordinador = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	intervalo_dump = config_get_int_value(config,"INTERVALO_DUMP");

	ip_config_coordinador = malloc(sizeof(char) * 100);
	strcpy(ip_config_coordinador,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	algoritmo_reemplazo = malloc(sizeof(char) * 100);
	strcpy(algoritmo_reemplazo,config_get_string_value(config, "ALGORITMO_REEMPLAZO"));

	punto_montaje = malloc(sizeof(char) * 100);
	strcpy(punto_montaje,config_get_string_value(config, "PUNTO_MONTAJE"));


	nombre_instancia = malloc(sizeof(char) * 100);
	strcpy(nombre_instancia,config_get_string_value(config, "NOMBRE_INSTANCIA"));

	config_destroy(config);
}


void free_parametros_config(){

	free(ip_config_coordinador);
	free(algoritmo_reemplazo);
	free(punto_montaje);
	free(nombre_instancia);
}

void recibo_datos_entrada(int fd_coordinador){
	void* buffer = malloc(sizeof(int32_t)*2);
	int numbytes = 0;
	if ((numbytes = recv(fd_coordinador, &tamanio_entrada, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño de la entrada\n");
		exit(1);
	}
	if ((numbytes = recv(fd_coordinador, &cant_entrada, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la cantidad de entradas\n");
		exit(1);
	}

	free(buffer);
}

//esto para q el coordinador puedo crear su t_instancia
void envio_datos(int fd_coordinador){

	//revisar
	int32_t longitud_mensaje = strlen(nombre_instancia) + 1;
	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),nombre_instancia,longitud_mensaje);

	if (send(fd_coordinador, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		printf("No pude enviar mis datos al coordinador\n");
		exit(1);
	}
	printf("Envie mi nombre correctamente\n");
	free(bufferEnvio);
}
