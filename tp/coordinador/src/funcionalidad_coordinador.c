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

	t_Instancia* instancia ;


	switch (keyfromstring(algoritmo)) {

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
			envio_tarea_instancia(2,instancia,2);
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
char ** get_clave_valor(int fd_esi) {

		int leng_clave = 0;
		int leng_valor = 0;
		int numbytes = 0;
		if ((numbytes = recv(fd_esi, &leng_clave, sizeof(int32_t), 0)) == -1) {
			printf("No se pudo recibir el tamaño de la clave\n");
			//MUERO
			exit(1);
		}
		char* clave = malloc(sizeof(char) * leng_clave);
		if ((numbytes = recv(fd_esi, clave, sizeof(char) * leng_clave, 0))
				== -1) {
			printf("No se pudo recibir la clave\n");
			//MUERO
			exit(1);
		}

		if ((numbytes = recv(fd_esi, &leng_valor, sizeof(int32_t), 0)) == -1) {
			printf("No se pudo recibir el tamaño del valor\n");
			//MUERO
			exit(1);
		}
		char* valor = malloc(sizeof(char) * leng_valor);
		if ((numbytes = recv(fd_esi, valor, sizeof(char) * leng_valor, 0))
				== -1) {
			printf("No se pudo recibir la valor\n");
			//MUERO
			exit(1);
		}
		printf("Recibi clave: %s valor: %s correctamente\n", clave, valor);

		char ** resultado = malloc(sizeof(char*) * 2);
		resultado[0] = clave;
		resultado[1] = valor;

		return resultado;
	}

void envio_tarea_instancia(int32_t id_operacion, t_Instancia * instancia,
			int32_t id_esi) {
		//todo: mirar de la cola de instancias cual seguiria y armar el buffer para mandar los datos
		char ** clave_valor_recibido = get_clave_valor(instancia->fd);
		loggeo_info(id_operacion, id_esi, clave_valor_recibido[0],
				clave_valor_recibido[1]);

		int32_t claveInstacia = strlen(clave_valor_recibido[0] + 1); // Tomo el CLAVE de la sentencia SET q me llega de la instacia
		int32_t valorInstacia = strlen(clave_valor_recibido[1]) + 1; // Tomo la VALOR  de la sentencia SET q me llega de la instacia

		void* bufferEnvio = malloc(
				sizeof(int32_t) * 2 + valorInstacia + claveInstacia);
		memcpy(bufferEnvio, &claveInstacia, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), &clave_valor_recibido[0],
				claveInstacia);
		memcpy(bufferEnvio + (sizeof(int32_t)) + claveInstacia, &valorInstacia,
				sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 2) + valorInstacia,
				clave_valor_recibido[1], valorInstacia);

		if (send(instancia->fd, bufferEnvio,
				sizeof(int32_t) * 2 + valorInstacia + claveInstacia, 0) == -1) {
			printf("No se pudo enviar la info a la INSTANCIA\n");
			free(bufferEnvio);
			exit(1);
		} else {
			printf(
					"Se envio un SET con CLAVE: %s y VALOR: %s a la INSTANCIA correctamente\n",
					clave_valor_recibido[0], clave_valor_recibido[1]);
		}

		free(clave_valor_recibido[0]);
		free(clave_valor_recibido[1]);
		free(clave_valor_recibido);
	}

void loggeo_info(int32_t id_operacion, int32_t id_esi, char* clave_recibida,
			char* valor_recibida) {
		char* registro = malloc(sizeof(char) * 500);
		strcpy(registro, "ESI ");
		strcat(registro, string_itoa(id_esi));
		switch (id_operacion) {
		case 1:
			//GET q tiene CLAVE
			strcat(registro, " GET ");
			strcat(registro, clave_recibida);
			break;
		case 2:
			//SET  q tiene CLAVE VALOR
			strcat(registro, " SET ");
			strcat(registro, clave_recibida);
			strcat(registro, " ");
			strcat(registro, valor_recibida);
			break;
		case 3:
			//STORE q tiene CLAVE
			strcat(registro, " STORE ");
			strcat(registro, clave_recibida);
			break;

		default:
			break;
		}
		log_info(LOGGER, registro);
		free(registro);
	}

void envio_tarea_planificador(int32_t id_operacion, char* clave_recibida,
			int32_t id_esi) {

		loggeo_info(id_operacion, id_esi, clave_recibida, "");
		int32_t len_clave = strlen(clave_recibida) + 1;

		//envio: ID_OPERACION,ID_ESI,LENG_CLAVE,CLAVE
		void* bufferEnvio = malloc(
				sizeof(int32_t) * 3 + sizeof(char) * len_clave);
		memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), &id_esi, sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 2), &len_clave,
				sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 3), clave_recibida,
				sizeof(char) * len_clave);

		if (send(FD_PLANIFICADOR, bufferEnvio,
				(sizeof(int32_t) * 3) + sizeof(char) * len_clave, 0) == -1) {
			printf("No se pudo enviar la info al planificador\n");
			free(clave_recibida);
			free(bufferEnvio);
			exit(1);
		} else {
			printf(
					"Se envio la tarea con clave: %s ID de ESI:%d al PLANIFICADOR correctamente\n",
					clave_recibida, id_esi);
		}

		free(clave_recibida);
		free(bufferEnvio);
	}


