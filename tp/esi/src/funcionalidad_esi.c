#include "funcionalidad_esi.h"

void free_parametros_config(){
	free(IP_CONFIG_COORDINADOR);
	free(IP_CONFIG_PLANIFICADOR);
}

void get_parametros_config() {
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//Mato ESI
		exit(1);
	}

	PUERTO_CONFIG_COORDINADOR = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");
	IP_CONFIG_COORDINADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_COORDINADOR,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	PUERTO_CONFIG_PLANIFICADOR = config_get_int_value(config,"PUERTO_CONFIG_PLANIFICADOR");
	IP_CONFIG_PLANIFICADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_PLANIFICADOR,config_get_string_value(config, "IP_CONFIG_PLANIFICADOR"));

	config_destroy(config);
}
