#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char *argv[])
{
	//-------------ARCHIVO DE LOG-------------------------

	//NO HACE FALTA LIBERAR MEMORIA SI LA FUNCION YA DEVUELVE UNA INSTANCIA LIBERADA
	t_log* loggers = log_create("logger.log","proyecto.c", false , LOG_LEVEL_INFO);
	


	log_info(loggers,"ANTES DE LIBERAR MEMORIA DEL MENSAJE");

	//PODES DAR MAS ESPACIAL DEL QUE USARIAS, TOTAL LO LIBERAS TODO LUEGO
	char *mensaje = malloc(sizeof(char)*10);
	strcpy(mensaje, "12345");
	
	printf("%s\n", mensaje);
	
	free(mensaje);
	

	log_info(loggers,"DESPUES DE LIBERAR MEMORIA DE MENSAJE");

	//ESTE INTERNAMENTE LIBERA TODO
	log_destroy(loggers);


	//------------ARCHIVO CONFIG-----------------------
	
	//busca y apunta al archivo config
	t_config* config = config_create("config.cfg");
	
	//imprime el valor correspondiente a clave IP
	printf("ANTES DEL CAMBIO: %s\n",config_get_string_value(config,"IP"));

	//cambia valor a la clave IP
	config_set_value(config,"IP","1.1.1.1");
	
	//guarda el cambio
	config_save(config);
	
	printf("DESPUES DEL CAMBIO: %s\n",config_get_string_value(config,"IP"));

	config_destroy(config);

	return 0;
}