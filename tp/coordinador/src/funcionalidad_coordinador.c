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
	pthread_mutex_lock(&MUTEX);
	bool _existInstancia(t_Instancia* una_instancia) { return strcmp(una_instancia->nombre_instancia,instanciaNueva->nombre_instancia)== 0;}
	if(list_find(LIST_INSTANCIAS, (void*)_existInstancia) != NULL){
		pthread_mutex_unlock(&MUTEX);
		return true;
	}
	pthread_mutex_unlock(&MUTEX);
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

	pthread_mutex_lock(&MUTEX);
	list_add(list,instancia_nueva);
	printf("Se agrego la instancia de nombre:%s a la lista\n",instancia_nueva->nombre_instancia);
	pthread_mutex_unlock(&MUTEX);
}

// retorna -> 1: si esta mal ; 2: si esta bien
int aplicarAlgoritmoDisctribucion(char * algoritmo,char** resultado){
	//TODO: revisar algoritmo porque solo toma SIEMPRE a la primera INSTANCIA,posiblemente "index" tenga q ser VG para persistir
	t_Instancia* inst;
	int index;
	if (strstr(algoritmo, "EL") != NULL) {
		index = 0;
		if(index == list_size(LIST_INSTANCIAS)){
			index = 0;
			inst = list_get(LIST_INSTANCIAS,index);
		}else{
			printf("Aplico Algoritmo EL\n");
			inst = list_get(LIST_INSTANCIAS,index);
			index ++;
		}
		return envio_tarea_instancia(2,inst,2,resultado);
	}
	if (strstr(algoritmo, "LSU") != NULL) {
		printf("INFO: Algoritmo LSU\n");
		//return envio_tarea_instancia(2,inst,2,resultado);
	}
	if (strstr(algoritmo, "INS") != NULL) {
		printf("INFO: Algoritmo KE\n");
		//return envio_tarea_instancia(2,inst,2,resultado);
	}


	return 1;
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

int envio_tarea_instancia(int32_t id_operacion, t_Instancia * instancia,
			int32_t id_esi,char** clave_valor_recibido) {
		//todo: mirar de la cola de instancias cual seguiria y armar el buffer para mandar los datos
		loggeo_info(id_operacion, id_esi, clave_valor_recibido[0],clave_valor_recibido[1]);

		int32_t claveInstacia = strlen(clave_valor_recibido[0]) + 1; // Tomo el CLAVE de la sentencia SET q me llega de la instacia
		int32_t valorInstacia = strlen(clave_valor_recibido[1]) + 1; // Tomo la VALOR  de la sentencia SET q me llega de la instacia

		void* bufferEnvio = malloc(sizeof(int32_t) * 2 + valorInstacia + claveInstacia);
		memcpy(bufferEnvio, &claveInstacia, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t),clave_valor_recibido[0],claveInstacia);
		memcpy(bufferEnvio + sizeof(int32_t) + claveInstacia, &valorInstacia,sizeof(int32_t));
		memcpy(bufferEnvio + (sizeof(int32_t) * 2) + claveInstacia,clave_valor_recibido[1], valorInstacia);

		if (send(instancia->fd, bufferEnvio,
				sizeof(int32_t) * 2 + valorInstacia + claveInstacia, 0) == -1) {
			printf("No se pudo enviar la info a la INSTANCIA\n");
			free(bufferEnvio);
			exit(1);
		} else {
			printf("Se envio SET clave: %s valor: %s a la INSTANCIA correctamente\n",clave_valor_recibido[0], clave_valor_recibido[1]);
		}

		free(clave_valor_recibido[0]);
		free(clave_valor_recibido[1]);
		free(clave_valor_recibido);

		return reciboRespuestaInstancia(instancia);
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

int reciboRespuestaInstancia(t_Instancia * instancia){

	int32_t respuestaInstacia = 0;
	int32_t numbytes = 0;
	//1:falle 2:ok
	if ((numbytes = recv(instancia->fd, &respuestaInstacia, sizeof(int32_t), 0)) <= 0) {
		if (numbytes == 0) {
		// conexión cerrada
			printf("Se fue el INSTANCIA: %s\n",instancia->nombre_instancia);
			return 1;
		} else {
			perror("ERROR: al recibir respuesta de la INSTANCIA");
			return 1;
		}
	}
	printf("Recibimos respuesta de Instancia: %s\n", instancia->nombre_instancia);
	return respuestaInstacia;
}


