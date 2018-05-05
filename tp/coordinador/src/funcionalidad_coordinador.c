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
  LOGGER = log_create("log de operaciones.log","tp-redistinto",0,LOG_LEVEL_INFO);
  log_info(LOGGER, "Empezamos.....");

}

void inicializo_semaforos(){
	pthread_mutex_init(&MUTEX,NULL);
	pthread_cond_init(&CONDICION_LIBERO_PLANIFICADOR, NULL);
	pthread_cond_init(&CONDICION_RECV_INSTANCIA, NULL);
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
	char* nombreInstancia = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fd_instancia, nombreInstancia, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		pthread_exit(NULL);
	}
	strcpy(instancia_nueva->nombre_instancia,nombreInstancia);

	instancia_nueva->nombre_instancia[strlen(instancia_nueva->nombre_instancia)]='\0';
	instancia_nueva->tamanio_libre = TAMANIO_ENTRADA * CANTIDAD_ENTRADAS;

	free(nombreInstancia);
	printf("Se creo la instancia de nombre:%s\n",instancia_nueva->nombre_instancia);
	return (instancia_nueva);
}

bool controlo_existencia(t_Instancia * instanciaNueva){

	bool _existInstancia(t_Instancia* una_instancia) { return strcmp(una_instancia->nombre_instancia,instanciaNueva->nombre_instancia)== 0;}
	if(list_find(LIST_INSTANCIAS, (void*)_existInstancia) != NULL){
		return true;
	}
	return false;
}

void send_mensaje_rechazo(t_Instancia * instancia_nueva){
	int32_t rechazo = 1;
	if (send(instancia_nueva->fd, &rechazo,sizeof(int32_t), 0) == -1) {
		printf("No se pudo enviar rechazo a la INSTANCIA\n");
	} else {
		printf("Se envio rechazo de NOMBRE existente para la INSTANCIA: %s\n",
				instancia_nueva->nombre_instancia);
	}
}

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva){

	//Aviso a la instancia q si lo pude agregar sin problema
	int32_t ok = 2; //instancia entiende q 1 es rechazo, cualquier otro valor es OK
	if (send(instancia_nueva->fd, &ok,sizeof(int32_t), 0) == -1) {
		printf("No se pudo enviar aceptacion a la INSTANCIA\n");
	} else {
		printf("Se envio aceptacion la INSTANCIA: %s\n",
				instancia_nueva->nombre_instancia);
	}

	list_add(list,instancia_nueva);
	printf("Se agrego la instancia de nombre:%s a la lista\n",instancia_nueva->nombre_instancia);

}

/*
int aplicarAlgoritmoDisctribucion(char * algoritmo,char** resultado){

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

	t_Instancia* instancia;

	switch (keyfromstring(algoritmo)) {

		int index;
		case EL:

			index = 0;
			if(index == list_size(LIST_INSTANCIAS)){
				index = 0;
				instancia = list_get(LIST_INSTANCIAS,index);
			}else{
				printf("Aplico Algoritmo EL\n");
				instancia = list_get(LIST_INSTANCIAS,index);
				index ++;
			}
			return envio_tarea_instancia(2,instancia,2,resultado);
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
	return 1;

}
*/


// retorna -> 1: si esta mal ; 2: si esta bien
int aplicarAlgoritmoDisctribucion(char * algoritmo,char** resultado){

	if (strstr(algoritmo, "EL") != NULL) {
		return equitativeLoad(resultado);
	}
	if (strstr(algoritmo, "LSU") != NULL) {
		printf("INFO: Algoritmo LSU\n");
		return LeastSpaceUsed(resultado);
		//return envio_tarea_instancia(2,inst,2,resultado);
	}
	if (strstr(algoritmo, "INS") != NULL) {
		printf("INFO: Algoritmo KE\n");
		//return envio_tarea_instancia(2,inst,2,resultado);
	}


	return FALLO_OPERACION_INSTANCIA;
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

int envio_recibo_tarea_store_instancia(int32_t id_operacion, char* clave,t_Instancia* instancia){

	int32_t len_clave = strlen(clave) + 1; // Tomo el CLAVE de la sentencia SET q me llega de la instacia

	void* bufferEnvio = malloc(sizeof(int32_t) * 2 + len_clave);
	memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
	memcpy(bufferEnvio+sizeof(int32_t), &len_clave, sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t)*2,clave,len_clave);

	if (send(instancia->fd, bufferEnvio,sizeof(int32_t) * 2 + len_clave, 0) == -1) {
		printf("No se pudo enviar la info a la INSTANCIA\n");
		free(bufferEnvio);
		RESULTADO_INSTANCIA_VG = FALLO_DESCONEXION_INSTANCIA; //para q esi sepa que falle
		return RESULTADO_INSTANCIA_VG;
	}
	printf("Se envio STORE clave: %s a la INSTANCIA correctamente\n",clave);

	//espero a la respuesta de la instancia (si es q la instancia esta) por 10 segundos
	struct timespec ts;
	struct timeval tp;
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += 10;
	pthread_cond_timedwait(&CONDICION_RECV_INSTANCIA,&MUTEX,&ts);
	//

	//pthread_cond_wait(&CONDICION_RECV_INSTANCIA,&MUTEX); //espero a la respuesta de la instancia (si es q la instancia esta)
	free(bufferEnvio);
	return RESULTADO_INSTANCIA_VG;
}

int envio_tarea_instancia(int32_t id_operacion, t_Instancia * instancia,int32_t id_esi,char** clave_valor_recibido) {
		//todo: mirar de la cola de instancias cual seguiria y armar el buffer para mandar los datos
		int32_t claveInstacia = strlen(clave_valor_recibido[0]) + 1; // Tomo el CLAVE de la sentencia SET q me llega de la instacia
		int32_t valorInstacia = strlen(clave_valor_recibido[1]) + 1; // Tomo la VALOR  de la sentencia SET q me llega de la instacia

		void* bufferEnvio = malloc(sizeof(int32_t) * 3 + valorInstacia + claveInstacia);
		memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
		memcpy(bufferEnvio+sizeof(int32_t), &claveInstacia, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t)*2,clave_valor_recibido[0],claveInstacia);
		memcpy(bufferEnvio + sizeof(int32_t)*2 + claveInstacia, &valorInstacia,sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 3) + claveInstacia,clave_valor_recibido[1], valorInstacia);

		if (send(instancia->fd, bufferEnvio,
				sizeof(int32_t) * 3 + valorInstacia + claveInstacia, 0) == -1) {
			printf("No se pudo enviar la info a la INSTANCIA\n");
			free(bufferEnvio);
			RESULTADO_INSTANCIA_VG = FALLO_OPERACION_INSTANCIA; //para q esi sepa que falle
			return RESULTADO_INSTANCIA_VG;
		}
		printf("Se envio SET clave: %s valor: %s a la INSTANCIA correctamente\n",clave_valor_recibido[0], clave_valor_recibido[1]);

		//espero a la respuesta de la instancia (si es q la instancia esta) por 10 segundos
		struct timespec ts;
		struct timeval tp;
		ts.tv_sec  = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec * 1000;
		ts.tv_sec += 10;
		pthread_cond_timedwait(&CONDICION_RECV_INSTANCIA,&MUTEX,&ts);
		//

		//pthread_cond_wait(&CONDICION_RECV_INSTANCIA,&MUTEX); //espero a la respuesta de la instancia (si es q la instancia esta) por 10 segundos

		if(RESULTADO_INSTANCIA_VG == OK_SET_INSTANCIA){
			//TODO: como hizo correctamente la operacion set, aca va
			//codigo para actualizar el espacio de memoria disponible de una instancia
			//buscandola en mi lista de registro instancia
		}
		free(bufferEnvio);
		free(clave_valor_recibido[0]);
		free(clave_valor_recibido[1]);
		free(clave_valor_recibido);
		return RESULTADO_INSTANCIA_VG;
}
void loggeo_respuesta(char* operacion, int32_t id_esi,int32_t resultado_linea){
	char* registro = malloc(sizeof(char) * 500);
	strcpy(registro, "ESI ");
	strcat(registro, string_itoa(id_esi));
	strcat(registro, " ");
	strcat(registro, operacion);
	strcat(registro, "  => ");
	switch (resultado_linea) {
	case ABORTA_ESI:
		strcat(registro, "ABORTA");
		break;
	case OK_ESI:
		strcat(registro, "OK");
		break;
	case BLOQUEADO_ESI:
		strcat(registro, "OK pero BLOQUEADO");
		break;

	default:
		strcat(registro, " - ");
		break;
	}
	log_info(LOGGER, registro);
	free(registro);
}

void loggeo_info(int32_t id_operacion, int32_t id_esi, char* clave_recibida,char* valor_recibida) {
	char* registro = malloc(sizeof(char) * 500);
	strcpy(registro, "ESI ");
	strcat(registro, string_itoa(id_esi));
	switch (id_operacion) {
	case GET:
		strcat(registro, " GET ");
		strcat(registro, clave_recibida);
		break;
	case SET:
		strcat(registro, " SET ");
		strcat(registro, clave_recibida);
		strcat(registro, " ");
		strcat(registro, valor_recibida);
		break;
	case STORE:
		strcat(registro, " STORE ");
		strcat(registro, clave_recibida);
		break;

	default:
		strcat(registro, " - ");
		break;
	}
	log_info(LOGGER, registro);
	free(registro);
}

void envio_tarea_planificador(int32_t id_operacion, char* clave_recibida,
			int32_t id_esi) {

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
			printf("Se envio la tarea con clave: %s ID de ESI:%d al PLANIFICADOR correctamente\n",clave_recibida, id_esi);
		}

		free(bufferEnvio);
	}


int reciboRespuestaInstancia(int fd_instancia){
	int32_t respuestaInstacia = 0;
	int32_t numbytes = 0;
	if ((numbytes = recv(fd_instancia, &respuestaInstacia, sizeof(int32_t), 0)) <= 0) {
		if (numbytes == 0) {
		// conexión cerrada
			printf("Se fue el INSTANCIA FD: %d, nos despedimos del hilo\n",fd_instancia);

		} else {
			perror("ERROR: al recibir respuesta de la INSTANCIA");

		}
		respuestaInstacia = FALLO_DESCONEXION_INSTANCIA;
	}else{
		printf("Recibimos respuesta de Instancia FD: %d\n", fd_instancia);

	}
	return respuestaInstacia;
}

void free_instancia(t_Instancia * instancia){
	free(instancia->nombre_instancia);
	free(instancia);
}


void remove_instancia(int fd_instancia){
	bool _esInstanciaFd(t_Instancia* instancia) { return instancia->fd == fd_instancia;}
	t_Instancia* unInstancia = list_find(LIST_INSTANCIAS,(void*)_esInstanciaFd);
	if(unInstancia != NULL){
		printf("Sacamos a la INSTANCIA: %s de la lista\n", unInstancia->nombre_instancia);
		list_remove_and_destroy_by_condition(LIST_INSTANCIAS,(void*) _esInstanciaFd,(void*) free_instancia);
	}
}

int equitativeLoad(char** resultado){
	t_Instancia* instancia;
	if(INDEX == list_size(LIST_INSTANCIAS)){
		INDEX = 0;
		instancia = list_get(LIST_INSTANCIAS,INDEX);//ojo q list_get si no encuentra nada retorna NULL
	}else{
		printf("Aplico Algoritmo EL\n");
		instancia = list_get(LIST_INSTANCIAS,INDEX);
		INDEX ++;
	}
	if(instancia != NULL){//pregunto si efectivamente hay algo
		return envio_tarea_instancia(2,instancia,2,resultado);
	}
	return FALLO_OPERACION_INSTANCIA;

}

int LeastSpaceUsed(char** resultado){
	int i;
	t_Instancia* instancia;
	t_Instancia* instancia_max;
		for(i=0;i<=list_size(LIST_INSTANCIAS);i++){
			if(i==0){
				instancia_max=list_get(LIST_INSTANCIAS,i);
			}else{
			instancia = list_get(LIST_INSTANCIAS,i);
			if((instancia_max->tamanio_libre) < (instancia->tamanio_libre)){
				instancia_max=list_get(LIST_INSTANCIAS,i);
				}
			}
		}
		if(instancia_max != NULL){//pregunto si efectivamente hay algo
			return envio_tarea_instancia(2,instancia_max,2,resultado);
		}
		return FALLO_OPERACION_INSTANCIA;

}
