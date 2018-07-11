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

void intHandler(int dummy) {
	if (dummy != 0) {
		char* msg = string_from_format("Finalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		logger_mensaje_error(msg);
		free(msg);
		exit(dummy);

	}
}

void configure_logger() {

	char* aux = string_from_format("log-ESI%d.log",ID_ESI_OBTENIDO);

	LOGGER = log_create(aux, "tp-redistinto", 1, LOG_LEVEL_INFO);

	free(aux);

	log_info(LOGGER, "Empezamos.....");
}

void logger_mensaje(char* mensaje) {
	log_info(LOGGER, mensaje);

}

void logger_mensaje_error(char* mensaje) {
	log_error(LOGGER, mensaje);

}
