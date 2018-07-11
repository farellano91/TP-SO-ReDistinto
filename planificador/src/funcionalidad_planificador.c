#include "funcionalidad_planificador.h"

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		char* msg = string_from_format("Finalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		logger_mensaje_error(msg);
		free(msg);
		log_destroy(LOGGER);
		exit(dummy);

	}
}

void configure_logger() {
	LOGGER = log_create("log-PLANIFICADOR.log","tp-redistinto",0,LOG_LEVEL_INFO);
	log_info(LOGGER, "Empezamos.....");
}

void logger_mensaje(char* mensaje) {
	pthread_mutex_lock(&MUTEX_LOGGER);
	log_info(LOGGER,mensaje);
	pthread_mutex_unlock(&MUTEX_LOGGER);
}

void logger_mensaje_error(char* mensaje) {
	pthread_mutex_lock(&MUTEX_LOGGER);
	log_error(LOGGER,mensaje);
	pthread_mutex_unlock(&MUTEX_LOGGER);
}

void free_claves_iniciales(){
	string_iterate_lines(CLAVES_INICIALES_BLOQUEADAS,(void*)free);
	free(CLAVES_INICIALES_BLOQUEADAS);
}

void free_parametros_config(){
	free(ALGORITMO_PLANIFICACION);
	free(IP_CONFIG_COORDINADOR);
	free_claves_iniciales();
}

void get_parametros_config(char* path) {
	//t_config* config = config_create("config.cfg");
	t_config* config = config_create(path);
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	PUERTO_ESCUCHA = config_get_int_value(config,"PUERTO_ESCUCHA");

    ALPHA = config_get_double_value(config,"ALPHA");

    ALPHA = ALPHA / 100;

    ALGORITMO_PLANIFICACION = malloc(sizeof(char) * 100);
	// HRRN , SJF-SD, SJF-CD
	strcpy(ALGORITMO_PLANIFICACION,config_get_string_value(config, "ALGORITMO_PLANIFICACION"));

	ESTIMACION_INICIAL = config_get_double_value(config,"ESTIMACION_INICIAL");

	IP_CONFIG_COORDINADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_COORDINADOR,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	IP_CONFIG_MIO = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_MIO,config_get_string_value(config, "IP_CONFIG_MIO"));

	PUERTO_CONFIG_COORDINADOR = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	PUERTO_CONFIG_COORDINADOR_STATUS = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR_STATUS");

//	CLAVES_INICIALES_BLOQUEADAS = malloc(sizeof(char*) * 100);
//	strcpy(CLAVES_INICIALES_BLOQUEADAS,config_get_string_value(config, "CLAVES_INICIALES_BLOQUEADAS"));
	CLAVES_INICIALES_BLOQUEADAS = config_get_array_value(config, "CLAVES_INICIALES_BLOQUEADAS");

	config_destroy(config);
}


t_Esi* creo_esi(t_respuesta_para_planificador respuesta,int32_t fd_esi){
	t_Esi* esi = malloc(sizeof(t_Esi));
	esi->lineaALeer = 0;
	esi->status = 2;
	esi->fd = fd_esi;  //lo necesito para luego saber a quien mandar el send
	esi->id = respuesta.id_esi;

	esi->tiempoEnListo = 0;
	esi ->estimacion = ESTIMACION_INICIAL;
	esi->cantSentenciasProcesadas = 0;
	return esi;
}

bool aplico_algoritmo_primer_ingreso(){
	//Aca ingreso un ESI nuevo, no le pedi que hiciera nada aun, asi que contadores no deberia actualizar
	//solo ordeno la lista en base a lo que tengo hasta ahora

	pthread_mutex_lock(&MUTEX);
	while (PLANIFICADOR_EN_PAUSA){
		//pthread_mutex_unlock(&MUTEX);
		pthread_cond_wait(&CONDICION_PAUSA_PLANIFICADOR,&MUTEX);
		//pthread_mutex_lock(&MUTEX);
	}
	pthread_mutex_unlock(&MUTEX);

	bool sContinuarComunicacion = true;

	ordeno_listas();

	//Pregunto si tengo alguno en LIST_EXECUTE (si esta vacio entro ya que significa q soy el unico)
	pthread_mutex_lock(&READY);
	pthread_mutex_lock(&EXECUTE);
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
		list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
		list_remove(LIST_READY, 0);
		t_Esi* unEsi = list_get(LIST_EXECUTE, 0);
		if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
			double rr = get_prioridad_HRRN(unEsi);
			char* aux = string_from_format("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f",unEsi->id,unEsi->tiempoEnListo,unEsi->estimacion,rr);
			logger_mensaje(aux);
			free(aux);
		}

	} else {
		sContinuarComunicacion = false;
	}
	pthread_mutex_unlock(&READY);
	pthread_mutex_unlock(&EXECUTE);
	return sContinuarComunicacion;

}
bool aplico_algoritmo_ultimo(){
	//si entro aca no estoy en exc, estoy en finish
	//Aca no actualizamos ningun contador ya que el ESI solo nos esta diciendo q no tiene nada para leer, asi que no deberia considerarse como
	//que esi hice una sentencia
	pthread_mutex_lock(&MUTEX);
	while (PLANIFICADOR_EN_PAUSA){
		//pthread_mutex_unlock(&MUTEX);
		pthread_cond_wait(&CONDICION_PAUSA_PLANIFICADOR,&MUTEX);
		//pthread_mutex_lock(&MUTEX);
	}
	pthread_mutex_unlock(&MUTEX);

	bool sContinuarComunicacion = true;
	pthread_mutex_lock(&EXECUTE);
	pthread_mutex_lock(&READY);
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
		pthread_mutex_unlock(&READY);
		ordeno_listas();
		pthread_mutex_lock(&READY);
		t_Esi* unEsiListo = list_get(LIST_READY, 0);
		list_add(LIST_EXECUTE, unEsiListo);
		list_remove(LIST_READY, 0);
		if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
			double rr = get_prioridad_HRRN(unEsiListo);
			char* aux = string_from_format("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f",unEsiListo->id,unEsiListo->tiempoEnListo,unEsiListo->estimacion,rr);
			logger_mensaje(aux);
			free(aux);
		}
	}
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&READY);
	//pregunto si hay alguien al menos
	pthread_mutex_lock(&EXECUTE);
	pthread_mutex_lock(&READY);
	if (list_is_empty(LIST_EXECUTE) && list_is_empty(LIST_READY)) {
		sContinuarComunicacion = false;
	}
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&READY);
	return sContinuarComunicacion;

}

//cuando sale de bloqueado a listo (sea con o sin desalojo, se recalcula igual)
void recalculo_estimacion(t_Esi *esi){
	esi->tiempoEnListo = 0;
//	if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") || strcmp(ALGORITMO_PLANIFICACION,"SJF-SD") == 0){
//		esi->estimacion = esi->estimacion*(1-ALPHA) + esi->cantSentenciasProcesadas*ALPHA;
//		esi->cantSentenciasProcesadas = 0;
//	}

	esi->estimacion = esi->estimacion*(1-ALPHA) + esi->cantSentenciasProcesadas*ALPHA;
	esi->cantSentenciasProcesadas = 0;

	char* aux = string_from_format("Esi %d tiene ahora una estimancion de %f",esi->id,esi->estimacion);
	logger_mensaje(aux);
	free(aux);
}


bool aplico_algoritmo(char clave[40]){
	//Aca estoy si recibi una respuesta de alguna tarea pedida al ESI (TAREAS QUE PUEDE SER REALIZADAS OK o BLOCKEADO
	//al ESI)

	pthread_mutex_lock(&MUTEX);//sirve para delimitar mi RC como atomica ya que usamos la misma variable entre hilos
	while (PLANIFICADOR_EN_PAUSA){
		//pthread_mutex_unlock(&MUTEX);
		pthread_cond_wait(&CONDICION_PAUSA_PLANIFICADOR,&MUTEX);
		//pthread_mutex_lock(&MUTEX);
	}
	pthread_mutex_unlock(&MUTEX);

	bool sContinuarComunicacion = true;
	pthread_mutex_lock(&EXECUTE);
	t_Esi* esiEjecutando = list_get(LIST_EXECUTE, 0);
	pthread_mutex_unlock(&EXECUTE);

	if(esiEjecutando != NULL){
		esiEjecutando ->cantSentenciasProcesadas++;
		char* aux = string_from_format("ESI %d tiene ahora %d sentencias hechas",esiEjecutando->id ,esiEjecutando ->cantSentenciasProcesadas);
		logger_mensaje(aux);
		free(aux);
	}

	//a todos los esis de ready les aumento 1 el tiempo de espera
	ActualizarIndicesEnLista();

	ordeno_listas();
	if(bloqueado_por_consola_flag()){
		pthread_mutex_lock(&EXECUTE);
		t_Esi* esi= list_get(LIST_EXECUTE, 0);
		pthread_mutex_unlock(&EXECUTE);
		//esi->lineaALeer --; //le resto 1 ya que quiro q cuando se desbloee vuelva a tratar de ejecutar la sentencia q lo bloqueo
		char* clave_block = malloc(sizeof(char)*40);
		strcpy(clave_block,CLAVE_BLOQUEO_CONSOLA);
		clave_block[strlen(clave_block)] = '\0';
		t_nodoBloqueado* esi_bloqueado = get_nodo_bloqueado(esi,clave_block);
		pthread_mutex_lock(&BLOCKED);
		list_add(LIST_BLOCKED,esi_bloqueado);
		pthread_mutex_unlock(&BLOCKED);
		free(clave_block);
		free(CLAVE_BLOQUEO_CONSOLA);
		desbloquea_flag(); //limpio el flag
		pthread_mutex_lock(&EXECUTE);
		list_remove(LIST_EXECUTE, 0);
		pthread_mutex_lock(&READY);
		list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
		list_remove(LIST_READY, 0);
		pthread_mutex_unlock(&EXECUTE);
		pthread_mutex_unlock(&READY);
		BlanquearIndices();
	}
		//controlo si tiene el flag de bloqueado para mandarlo a la list_block
	else if(bloqueado_flag() ==  1){
			//Aca tengo que actualizar la estimacion inicial anterior.
			//saco de EXECUTE a BLOQUEADO
			// sumo una sentencia mas procesada que seria el get
			pthread_mutex_lock(&EXECUTE);
			t_Esi* esi= list_get(LIST_EXECUTE, 0);
			pthread_mutex_unlock(&EXECUTE);
			esi->lineaALeer --; //le resto 1 ya que quiro q cuando se desbloee vuelva a tratar de ejecutar la sentencia q lo bloqueo
			char* clave_block = malloc(sizeof(char)*40);
			strcpy(clave_block,clave);
			clave_block[strlen(clave_block)] = '\0';
			t_nodoBloqueado* esi_bloqueado = get_nodo_bloqueado(esi,clave_block);
			pthread_mutex_lock(&BLOCKED);
			list_add(LIST_BLOCKED,esi_bloqueado);
			pthread_mutex_unlock(&BLOCKED);

			char* aux = string_from_format("Muevo de EJECUCION a BLOQUEADO al ESI ID:%d por la clave:%s",esi->id,clave_block);
			logger_mensaje(aux);
			free(aux);

			free(clave_block);


			//Caso donde ESI se bloqueo al hacer lo que le pedi
			desbloquea_flag(); //limpio el flag
			//Solo lo saco de EXEC (cuando supe que era bloqueado porque el coordinador me informo puse flag = 1 y copie de exec ->  bloqueado)
			pthread_mutex_lock(&EXECUTE);
			list_remove(LIST_EXECUTE, 0);
			//toma el primero de listo -> exec y lo saca de listo
			pthread_mutex_lock(&READY);
			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
			pthread_mutex_unlock(&EXECUTE);
			pthread_mutex_unlock(&READY);
			//Blanqueo el Esi que pasa a ejecutando
			BlanquearIndices();
		}else{

			//caso donde ESI hizo lo que le pidieron OK, no esta bloqueado pero el algoritmo es con desalojo
			//si entra aca significa que hizo la operacion, osea podemos contar++ ;)
			if (strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") == 0) {
				if(!list_is_empty(LIST_READY)){
					//Revisar , si la estimacion del primero es mayor al que actualmente tengo, no tengo que desalojar
					pthread_mutex_lock(&READY);
					t_Esi* esiReady = list_get(LIST_READY, 0);
					pthread_mutex_unlock(&READY);

					if( esiEjecutando->estimacion <= esiReady->estimacion){
					//if( (esiEjecutando->estimacion - esiEjecutando->cantSentenciasProcesadas) < esiReady->estimacion){
						return sContinuarComunicacion;
					}
					//exc -> listo
					char* aux = string_from_format("DESALOJAMOS al ESI %d por el ESI %d",esiEjecutando->id,esiReady->id);
					logger_mensaje(aux);
					free(aux);

					//esiEjecutando->estimacion = esiEjecutando->estimacion - esiEjecutando->cantSentenciasProcesadas;//la estimacion sigue siendo la misma
					pthread_mutex_lock(&EXECUTE);
					pthread_mutex_lock(&READY);
					list_add(LIST_READY, list_get(LIST_EXECUTE, 0));
					list_remove(LIST_EXECUTE, 0);
					//el primero de listo va a exec
					list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
					list_remove(LIST_READY, 0);
					pthread_mutex_unlock(&EXECUTE);
					pthread_mutex_unlock(&READY);
					//Blanqueo el Esi que pasa a ejecutando
					BlanquearIndices();
				}

			}else{
				//ACA estoy si el ESI hizo lo que le pedi OK sin bloquearse y tampoco hay desalojo
				//(si no esta bloqueado y no es con desalojo no hago nada, solo continuo la comunicacion con el,
				//no hace falta ordenar las lista ya q estas se ordenaran cuando se bloquee el ESI o cuando TERMINE)
	//			IncrementarLinealeer(list_get(LIST_EXECUTE, 0));
			}
		}
	return sContinuarComunicacion;

}

// Pongo en 0 el tiempo en listos del esi que voy a ejecutar.
void BlanquearIndices(){
	pthread_mutex_lock(&EXECUTE);
	t_Esi* esiEjecutando = list_get(LIST_EXECUTE, 0);
	if(esiEjecutando != NULL){

		if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
			double rr = get_prioridad_HRRN(esiEjecutando);
			char* aux = string_from_format("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f",esiEjecutando->id,esiEjecutando->tiempoEnListo,esiEjecutando->estimacion,rr);
			logger_mensaje(aux);
			free(aux);
		}

		esiEjecutando->tiempoEnListo = 0;
		//esiEjecutando->cantSentenciasProcesadas = 0;
	}
	pthread_mutex_unlock(&EXECUTE);
}
void ActualizarIndices(t_Esi *esi){
	esi->tiempoEnListo = esi->tiempoEnListo + 1;
	char* aux = string_from_format("ESI %d tiene ahora %d tiempo en listo",esi->id,esi->tiempoEnListo);
	logger_mensaje(aux);
	free(aux);
}

void ActualizarIndicesEnLista(){
	// recorro la lista de ready y aplico funcion actualizar Indices
	pthread_mutex_lock(&READY);
	if(!list_is_empty(LIST_READY)){
		list_iterate(LIST_READY,(void*)ActualizarIndices );
	}
	pthread_mutex_unlock(&READY);
}
//void IncrementarLinealeer(t_Esi *esi){
//	esi->lineaALeer++;
//}

void desbloquea_flag(){
	pthread_mutex_lock(&EXECUTE);
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	un_esi->status = 0;
	pthread_mutex_unlock(&EXECUTE);
}

bool bloqueado_flag(){
	pthread_mutex_lock(&EXECUTE);
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	pthread_mutex_unlock(&EXECUTE);
	return (un_esi->status == 1);
}

bool bloqueado_por_consola_flag(){
	pthread_mutex_lock(&EXECUTE);
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	pthread_mutex_unlock(&EXECUTE);
	return (un_esi->status == 4);
}

bool muerto_flag(){
	pthread_mutex_lock(&EXECUTE);
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	pthread_mutex_unlock(&EXECUTE);
	return (un_esi->status == 3);
}

//Ordena la lista de ready dependiendo del algoritmo que se usa
void ordeno_listas(){
	pthread_mutex_lock(&READY);

	//estado antes de ordenar
//	void mostrarA(t_Esi* reg){
//		printf("[ANTES DE ORDENAR ESI %d ESTIMACION %f]\n",reg->id,reg->estimacion);
//	}
//	list_iterate(LIST_READY,(void*)mostrarA );
//

	if ((strcmp(ALGORITMO_PLANIFICACION, "SJF-CD") == 0) || (strcmp(ALGORITMO_PLANIFICACION,"SJF-SD")==0)){
		order_list(LIST_READY, (void*) ordenar_por_estimacion);
	}
	if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
		order_list(LIST_READY, (void*) ordenar_por_prioridad);

	}

	//estado despues de ordenar
//	void mostrarD(t_Esi* reg){
//		printf("[DESPUES DE ORDENAR ESI %d ESTIMACION %f]\n",reg->id,reg->estimacion);
//	}
//	list_iterate(LIST_READY,(void*)mostrarD );

	pthread_mutex_unlock(&READY);
}

//Envia permiso de hacer una lectura a ESI
/*
 * flags_continuar = 1 OK
 *
 * */
void continuar_comunicacion(){

	pthread_mutex_lock(&EXECUTE);
	t_Esi * primer_esi = list_get(LIST_EXECUTE,0);
	if(primer_esi == NULL){
		list_remove(LIST_EXECUTE, 0);
	}else{
		primer_esi->lineaALeer ++;
		if (send(primer_esi->fd, &(primer_esi->lineaALeer), sizeof(int32_t), 0) == -1) {
			primer_esi->lineaALeer --;

			char* aux = string_from_format("Error al tratar de enviar el permiso a ESI");
			logger_mensaje_error(aux);
			free(aux);
		}else{
			char* aux = string_from_format("Envie permiso de ejecucion linea: %d al ESI de ID: %d ESTIMACION: %f",primer_esi->lineaALeer, primer_esi->id, primer_esi->estimacion);
			logger_mensaje(aux);
			free(aux);

		}
	}
	pthread_mutex_unlock(&EXECUTE);

}

// en base a este ejemplo: https://github.com/sisoputnfrba/ansisop-panel/blob/master/panel/kernel.c

void free_nodoBLoqueado(t_nodoBloqueado* nodoBloqueado){


	if (nodoBloqueado->clave)
		free(nodoBloqueado->clave);
	if (nodoBloqueado)
		free(nodoBloqueado);


}

//TODO:analizar si esto funciona, (si el esi esta liberado lo toma como NULL?)
void free_esiBloqueador(t_esiBloqueador* nodoBloqueado){


	if (nodoBloqueado->clave)
		free(nodoBloqueado->clave);
	if (nodoBloqueado)
		free(nodoBloqueado);

}

void remove_esi_by_fd(int32_t fd){
	bool _esElfd(t_Esi* un_esi) { return un_esi->fd == fd;}
	pthread_mutex_lock(&READY);
	list_remove_and_destroy_by_condition(LIST_READY,(void*) _esElfd, free);
	pthread_mutex_unlock(&READY);
	pthread_mutex_lock(&EXECUTE);
	list_remove_and_destroy_by_condition(LIST_EXECUTE,(void*) _esElfd, free);
	pthread_mutex_unlock(&EXECUTE);

	pthread_mutex_lock(&FINISHED);
	list_remove_and_destroy_by_condition(LIST_FINISHED,(void*) _esElfd, free);
	pthread_mutex_unlock(&FINISHED);

	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}
	pthread_mutex_lock(&BLOCKED);
	list_remove_and_destroy_by_condition(LIST_BLOCKED,(void*) _esElfdBlocked,(void*) free_nodoBLoqueado);
	pthread_mutex_unlock(&BLOCKED);

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	pthread_mutex_lock(&ESISBLOQUEADOR);
	list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);
	pthread_mutex_unlock(&ESISBLOQUEADOR);

}


//Buscara al esi que hizo ctrl+c y lo manda a terminado ()
void remove_esi_by_fd_finished(int32_t fd){
	bool _esElfd(t_Esi* un_esi) { return un_esi->fd == fd;}
	pthread_mutex_lock(&READY);
	if(list_find(LIST_READY, (void*)_esElfd) != NULL){
		t_Esi* esi_terminado = list_find(LIST_READY, (void*)_esElfd);
		//le resto uno ya que al ingresar a listo se le sumo uno y
		//ahora estaba ejecutando pero no hizo nada, termino de una pork no tenia nada mas para ejecutar, entonces le restamos eso asi
		//queda bien guardado en la lisa de finalizado
		pthread_mutex_lock(&FINISHED);
		list_add(LIST_FINISHED,esi_terminado);
		pthread_mutex_unlock(&FINISHED);

		list_remove_by_condition(LIST_READY,(void*) _esElfd);
	}
	pthread_mutex_unlock(&READY);
	pthread_mutex_lock(&EXECUTE);
	if(list_find(LIST_EXECUTE, (void*)_esElfd) != NULL){
		pthread_mutex_lock(&FINISHED);
		list_add(LIST_FINISHED,list_find(LIST_EXECUTE, (void*)_esElfd));
		pthread_mutex_unlock(&FINISHED);

		list_remove_by_condition(LIST_EXECUTE,(void*) _esElfd);
	}
	pthread_mutex_unlock(&EXECUTE);

	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}

	pthread_mutex_lock(&BLOCKED);
	if(list_find(LIST_BLOCKED, (void*)_esElfdBlocked) != NULL){
		t_nodoBloqueado * nodoBuscado = list_find(LIST_BLOCKED, (void*)_esElfdBlocked);
		pthread_mutex_lock(&FINISHED);
		list_add(LIST_FINISHED,nodoBuscado->esi);
		pthread_mutex_unlock(&FINISHED);
		list_remove_by_condition(LIST_BLOCKED,(void*) _esElfdBlocked);
	}
	pthread_mutex_unlock(&BLOCKED);
	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}

	pthread_mutex_lock(&ESISBLOQUEADOR);
	if(list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador) != NULL){
		t_esiBloqueador * esiBloqueador = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		pthread_mutex_lock(&FINISHED);
		list_add(LIST_FINISHED,esiBloqueador->esi);
		pthread_mutex_unlock(&FINISHED);
		list_remove_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador);
	}
	pthread_mutex_unlock(&ESISBLOQUEADOR);


}

//libera solo el recurso mas no al esi bloqueado si hubiese
void free_only_recurso(int32_t fd){

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	int32_t cant_esis_borrar = 0;
	pthread_mutex_lock(&ESISBLOQUEADOR);
	if(list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador) != NULL){
		cant_esis_borrar = list_count_satisfying(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
	}
	int32_t contador = 0;
	while (contador < cant_esis_borrar){

		t_esiBloqueador * eb = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		char* aux = string_from_format("Libero clave:%s de ESI ID:%d", eb->clave,eb->esi->id);
		logger_mensaje(aux);
		free(aux);

		list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);
		contador++;
	}
	pthread_mutex_unlock(&ESISBLOQUEADOR);
}


//libero tooodos los get clave que tenia tomado el esi de fd
void free_recurso(int32_t fd){

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	int32_t cant_esis_borrar = 0;
	pthread_mutex_lock(&ESISBLOQUEADOR);
	if(list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador) != NULL){
		cant_esis_borrar = list_count_satisfying(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
	}
	int32_t contador = 0;
	while (contador < cant_esis_borrar){

		t_esiBloqueador * eb = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		char* aux = string_from_format("Libero clave:%s de ESI ID:%d", eb->clave,eb->esi->id);
		logger_mensaje(aux);
		free(aux);

		//paso de bloqueado a listo (EL PRIMER) ESIs que esperaban esa clave
		move_esi_from_bloqueado_to_listo(eb->clave);
		list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);
		contador++;
	}
	pthread_mutex_unlock(&ESISBLOQUEADOR);
}

//paso de bloqueado a listo (EL PRIMER) los esis que pedian esa clave
void move_esi_from_bloqueado_to_listo(char* clave){

	//No se ordena la listas de listos porque se re ordena cuando actualizo los incides.
	bool _esElid(t_nodoBloqueado* nodoBloqueado) { return (strcmp(nodoBloqueado->clave,clave)==0);}
	int32_t cant_esis_mover = 0;

	pthread_mutex_lock(&BLOCKED);
	if(list_find(LIST_BLOCKED, (void*)_esElid) != NULL){
		cant_esis_mover = list_count_satisfying(LIST_BLOCKED, (void*)_esElid);
	}
	if(cant_esis_mover>0){
		t_nodoBloqueado* nodoBloqueado = list_find(LIST_BLOCKED,(void*) _esElid);
		list_remove_by_condition(LIST_BLOCKED,(void*) _esElid);
		t_Esi* esi = nodoBloqueado->esi;
		esi->tiempoEnListo = 0;
		pthread_mutex_lock(&READY);
		list_add(LIST_READY,esi);
		pthread_mutex_unlock(&READY);
		char* aux = string_from_format("Desbloqueo al ESI ID:%d ya que esperaba la clave: %s", esi->id,nodoBloqueado->clave);
		logger_mensaje(aux);
		free(aux);
		recalculo_estimacion(esi);
		free(nodoBloqueado);
	}else{
		char* aux = string_from_format("No hay ningun ESI para desbloquear por la clave: %s",clave);
		logger_mensaje(aux);
		free(aux);
	}
	ordeno_listas();
	pthread_mutex_unlock(&BLOCKED);

}

t_list* create_list(){
	t_list * Lready = list_create();

	return Lready;
}

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, char* clave){

	t_nodoBloqueado* nodoBloqueado = malloc(sizeof(t_nodoBloqueado));
//	nodoBloqueado->esi = malloc(sizeof(t_Esi));
	int32_t len = strlen(clave) + 1;
	nodoBloqueado->clave = malloc(sizeof(char)* len);
	strcpy(nodoBloqueado->clave,clave);
	nodoBloqueado->esi = esi;


	return nodoBloqueado;
}

//creo t_esiBloqueador usando un esi (que no deberia liberarlo aun) y su clave
t_esiBloqueador* get_esi_bloqueador(t_Esi* esi, char* clave){

	t_esiBloqueador* esiBloqueador = malloc(sizeof(t_esiBloqueador));
//	esiBloqueador->esi = malloc(sizeof(t_Esi));

	esiBloqueador->esi = esi;
	int32_t len = strlen(clave) + 1;
	esiBloqueador->clave = malloc(sizeof(char)* len);
	strcpy(esiBloqueador->clave,clave);

	return esiBloqueador;
}

// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. OK
double get_prioridad_HRRN(t_Esi* esi){
	if( esi == NULL){
		return 0;
	}
	double result =  ( esi->tiempoEnListo +  esi->estimacion ) / esi->estimacion;
	return result;
}

/*------------------------------ORDENAMIENTO SIN DESALOJO--------------------------*/
//Ordena mi list (seria sobre mi lista de ready)
void order_list(t_list* lista, void * funcion){
	list_sort(lista, (void*) funcion);
}

bool ordenar_por_estimacion(t_Esi * esi_menor, t_Esi * esi) {
	return ((esi_menor->estimacion) <= (esi->estimacion)); //agrego el = pork pasaba q para el mismo valor lo cambiaba de lugar cosa q estaba de mas
}

bool ordenar_por_prioridad(t_Esi * esi_menor, t_Esi * esi) {
	return (get_prioridad_HRRN(esi_menor) >= get_prioridad_HRRN(esi));
}

void agregar_en_bloqueados(t_Esi *esi, char* clave){
	t_nodoBloqueado* nodoBloqueado = get_nodo_bloqueado(esi,clave);
	pthread_mutex_lock(&BLOCKED);
	list_add(LIST_BLOCKED,nodoBloqueado);
	pthread_mutex_unlock(&BLOCKED);
}

//Tanto para lista de listos como para la de finalizados.
void agregar_en_Lista(t_list* lista, t_Esi *esi){
//	esi->lineaALeer ++;
	list_add( lista ,esi);
}
//TODO: criterio de ordenamiento SJF con DESALOJO

//Mueve mi esi de una lista a otra
void cambio_de_lista(t_list* list_desde,t_list* list_hasta, int32_t id_esi){

	bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
	t_Esi* esi_buscado = list_find(list_desde,(void*) _esElid);

	list_remove_by_condition(list_desde,(void*) _esElid);
	agregar_en_Lista(list_hasta,esi_buscado);
}

void cambio_ejecutando_a_finalizado(int32_t id_esi){

	bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
	pthread_mutex_lock(&EXECUTE);
	t_Esi* esi_buscado = list_find(LIST_EXECUTE,(void*) _esElid);
	list_remove_by_condition(LIST_EXECUTE,(void*) _esElid);
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_lock(&FINISHED);
	agregar_en_Lista(LIST_FINISHED,esi_buscado);
	pthread_mutex_unlock(&FINISHED);
}

void inicializo_semaforos(){
	pthread_mutex_init(&MUTEX,NULL);
	pthread_mutex_init(&BLOCKED,NULL);
	pthread_mutex_init(&READY,NULL);
	pthread_mutex_init(&EXECUTE,NULL);
	pthread_mutex_init(&SOCKETS,NULL);
	pthread_mutex_init(&ESISBLOQUEADOR,NULL);
	pthread_mutex_init(&FINISHED,NULL);
	pthread_cond_init(&CONDICION_PAUSA_PLANIFICADOR, NULL);
	pthread_mutex_init(&MUTEX_LOGGER,NULL);

}

void move_esi_from_ready_to_finished(int id){
	bool _esElid(t_Esi* un_esi) { return un_esi->id == id;}
	pthread_mutex_lock(&READY);
	t_Esi* esi_buscado = list_find(LIST_READY,(void*) _esElid);
	list_remove_by_condition(LIST_READY,(void*) _esElid);
	pthread_mutex_unlock(&READY);
	pthread_mutex_lock(&FINISHED);
	agregar_en_Lista(LIST_FINISHED,esi_buscado);
	pthread_mutex_unlock(&FINISHED);
}

bool quiereAlgoQueElOtroTiene(t_esiBloqueador* esiBloqueador, t_nodoBloqueado* nodo){

	bool _es_el_esi1(t_nodoBloqueado* un_nodo){
		return un_nodo->esi->id == esiBloqueador->esi->id;
	}

	bool _es_el_esi2(t_esiBloqueador* esi_bloqueador){
		return esi_bloqueador->esi->id == nodo->esi->id;
	}

	t_nodoBloqueado* un_esi = list_find(LIST_BLOCKED, (void*) _es_el_esi1);;

	pthread_mutex_lock(&ESISBLOQUEADOR);
	t_esiBloqueador* otro_esi = list_find(LIST_ESI_BLOQUEADOR, (void*) _es_el_esi2);
	pthread_mutex_unlock(&ESISBLOQUEADOR);

	return string_equals_ignore_case(un_esi->clave, otro_esi->clave);

}
