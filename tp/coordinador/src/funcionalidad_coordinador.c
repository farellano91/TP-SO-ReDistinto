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

	PUERTO_ESCUCHA_CONEXION = config_get_int_value(config,"PUERTO_ESCUCHA_CONEXION");

	ALGORITMO_DISTRIBUCION = malloc(sizeof(char) * 100);
	strcpy(ALGORITMO_DISTRIBUCION,config_get_string_value(config, "ALGORITMO_DISTRIBUCION"));

	CANTIDAD_ENTRADAS = config_get_int_value(config,"CANTIDAD_ENTRADAS");
	TAMANIO_ENTRADA = config_get_int_value(config,"TAMANIO_ENTRADA");
	RETARDO = config_get_int_value(config,"RETARDO");

	config_destroy(config);
}

//libera todos los parametros que tenga
void free_parametros_config(){

	free(ALGORITMO_DISTRIBUCION);
}

void configure_logger() {
  LOGGER = log_create("log de operaciones.log","tp-redistinto",1,LOG_LEVEL_INFO);
  log_info(LOGGER, "Empezamos.....");

}

void inicializo_semaforos(){
	pthread_mutex_init(&MUTEX,NULL);
	pthread_cond_init(&CONDICION_LIBERO_PLANIFICADOR, NULL);

}

t_list* create_list(){
	t_list * Lready = list_create();
	return Lready;
}

void envio_datos_entrada(int fd_instancia){
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&TAMANIO_ENTRADA,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&CANTIDAD_ENTRADAS,sizeof(int32_t));

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
	instancia_nueva->tamanio_libre = TAMANIO_ENTRADA * CANTIDAD_ENTRADAS;

	free(mensajeSaludoRecibido);
	printf("Se creo la instancia de nombre:%s\n",instancia_nueva->nombre_instancia);
	return (instancia_nueva);
}

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva){

	pthread_mutex_lock(&MUTEX);
	list_add(list,instancia_nueva); //esto lo encola al final
	printf("Se agrego la instancia de nombre:%s a la lista\n",instancia_nueva->nombre_instancia);
	pthread_mutex_unlock(&MUTEX);
}

void aplicarAlgoritmoDisctribucion(char * algoritmo){

	#define INVALID_ALGORITMO_DISTRIBUCION -1
	#define EL 4
	#define LSU 5
	#define KE 6

	typedef struct { char *key; int val; } t_symstruct;

	static t_symstruct buscarTabla[] = {
		{ "EL", EL }, { "LSU", LSU }, { "KE", KE }
	};

	#define NKEYS (sizeof(buscarTabla)/sizeof(t_symstruct))

	int keyfromstring(char *key)
	{
		int i;
		for (i=0; i < NKEYS; i++) {
			t_symstruct *sym = buscarTabla + i*sizeof(t_symstruct);
			if (strcmp(sym->key, key) == 0)
				return sym->val;
		}
		return INVALID_ALGORITMO_DISTRIBUCION;
	}

	switch (keyfromstring(algoritmo)) {
		t_Instancia* instancia ;
		int index;
		case EL:
			index = 0;
			if(index==list_size(LIST_INSTANCIAS)){
				index = 0;
			}else{
				index ++;
			}
			instancia = list_get(LIST_INSTANCIAS,index);
			//TODO:Implementar t_id_esi;
			envio_tarea_instancia(2,get_clave_valor(instancia->fd),2);
			printf("INFO: Algoritmo EL\n");
			break;
		case LSU:
			printf("INFO: Algoritmo LSU\n");
			break;
		case KE:
			printf("INFO: Algoritmo KE\n");
			break;
		case INVALID_ALGORITMO_DISTRIBUCION:
			printf("Error: ALGORITMO_DISTRIBUCION invalido\n");
			exit(1);

	}


}
