#include "funcionalidad_esi.h"

void free_parametros_config(){
	free(ip_config_coordinador);
	free(ip_config_planificador);
}

void get_parametros_config() {
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//Mato ESI
		exit(1);
	}

	puerto_config_coordinador = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");
	ip_config_coordinador = malloc(sizeof(char) * 100);
	strcpy(ip_config_coordinador,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	puerto_config_planificador = config_get_int_value(config,"PUERTO_CONFIG_PLANIFICADOR");
	ip_config_planificador = malloc(sizeof(char) * 100);
	strcpy(ip_config_planificador,config_get_string_value(config, "IP_CONFIG_PLANIFICADOR"));

	config_destroy(config);
}
