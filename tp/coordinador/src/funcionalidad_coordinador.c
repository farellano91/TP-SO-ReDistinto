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
