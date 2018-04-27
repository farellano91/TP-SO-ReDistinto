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
	RETARDO = config_get_int_value(config,"RETARDO");

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

void inicializo_semaforos(){
	 pthread_mutex_init(&mutex, NULL);
	 pthread_cond_init(&CONDICION_LIBERO_PLANIFICADOR, NULL);

}

t_list* create_list(){
	t_list * Lready = list_create();
	return Lready;
}

void envio_datos_entrada(int fd_instancia){
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&tamanio_entrada,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&cantidad_entradas,sizeof(int32_t));

	if (send(fd_instancia, bufferEnvio,sizeof(int32_t)*2, 0) == -1) {
		printf("No se pudo enviar datos de entrada a la instancia\n");
		//muere hilo
		pthread_exit(NULL);
	}
	printf("Datos de entrada enviado correctamente\n");
	free(bufferEnvio);
}

t_Instancia* creo_instancia(int fd_instancia){
	t_Instancia* instancia_nueva = malloc(sizeof(t_Instancia));
	instancia_nueva->nombre_instancia = malloc(sizeof(char)*100);

	instancia_nueva->fd = fd_instancia;

	//Recibo int:longitud nombre y char*: nombre
	int32_t longitud = 0;
	int numbytes = 0;
	if ((numbytes = recv(fd_instancia, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño del nombre de la instancia\n");
		pthread_exit(NULL);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fd_instancia, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		pthread_exit(NULL);
	}
	strcpy(instancia_nueva->nombre_instancia,mensajeSaludoRecibido);

	instancia_nueva->nombre_instancia[strlen(instancia_nueva->nombre_instancia)]='\0';
	instancia_nueva->tamanio_libre = tamanio_entrada * cantidad_entradas;

	free(mensajeSaludoRecibido);
	printf("Se creo la instancia de nombre:%s\n",instancia_nueva->nombre_instancia);
	return (instancia_nueva);
}

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva){

	pthread_mutex_lock(&mutex);
	list_add(list,instancia_nueva); //esto lo encola al final
	printf("Se agrego la instancia de nombre:%s a la lista\n",instancia_nueva->nombre_instancia);
	pthread_mutex_unlock(&mutex);
}
