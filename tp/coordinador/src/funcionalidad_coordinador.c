/*
 * funcionalidad_coordinador.c
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */
#include "funcionalidad_coordinador.h"

void get_parametros_config(){

	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	puerto_escucha_conexion = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION");

	algoritmo_distribucion = malloc(sizeof(char) * 100);
	strcpy(algoritmo_distribucion,config_get_string_value(config, "ALGORITMO_DISTRIBUCION"));

	cantidad_entradas = config_get_int_value(config,"CANTIDAD_ENTRADAS");
	tamanio_entrada = config_get_int_value(config,"TAMANIO_ENTRADA");
	retardo = config_get_int_value(config,"RETARDO");

	config_destroy(config);
}

//libera todos los parametros que tenga
void free_parametros_config(){

	free(algoritmo_distribucion);
}

void configure_logger() {
  logger = log_create("log de operaciones.log","tp-redistinto",1,LOG_LEVEL_INFO);
  log_info(logger, "Empezamos.....");

}


t_list* create_list_instancias(){
	t_list * Lready = list_create();
	return Lready;
}

void envio_datos_entrada(int fd_instancia){
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&tamanio_entrada,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&tamanio_entrada,sizeof(int32_t));

	if (send(fd_instancia, bufferEnvio,sizeof(int32_t)*2, 0) == -1) {
		printf("No se pudo enviar datos de entrada a la instancia\n");
		pthread_exit(NULL);
	}
	printf("Datos de entrada enviado correctamente\n");
	free(bufferEnvio);
}

t_Instancia* creo_instancia(int fd_instancia){
	t_Instancia instancia_nueva = malloc(sizeof(t_Instancia));

	return (instancia_nueva);
}

void agrego_instancia_lista(t_list* list_instancias,t_Instancia* instancia_nueva){
	list_add(list_instancias,instancia_nueva); //esto lo encola al final
}
