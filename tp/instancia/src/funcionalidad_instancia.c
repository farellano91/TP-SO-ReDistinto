
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
