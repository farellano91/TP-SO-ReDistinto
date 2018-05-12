#include "funcionalidad_planificador.h"


void free_claves_iniciales(){
	string_iterate_lines(CLAVES_INICIALES_BLOQUEADAS,(void*)free);
	free(CLAVES_INICIALES_BLOQUEADAS);
}

void free_parametros_config(){
	free(ALGORITMO_PLANIFICACION);
	free(IP_CONFIG_COORDINADOR);
	free_claves_iniciales();
}

void get_parametros_config() {
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	PUERTO_ESCUCHA = config_get_int_value(config,"PUERTO_ESCUCHA");

    ALPHA = config_get_double_value(config,"ALPHA");

    ALGORITMO_PLANIFICACION = malloc(sizeof(char) * 100);
	// HRRN , SJF, SJFD
	strcpy(ALGORITMO_PLANIFICACION,config_get_string_value(config, "ALGORITMO_PLANIFICACION"));

	ESTIMACION_INICIAL = config_get_int_value(config,"ESTIMACION_INICIAL");


	IP_CONFIG_COORDINADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_COORDINADOR,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	PUERTO_CONFIG_COORDINADOR = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	CLAVES_INICIALES_BLOQUEADAS = malloc(sizeof(char*) * 100);
//	strcpy(CLAVES_INICIALES_BLOQUEADAS,config_get_string_value(config, "CLAVES_INICIALES_BLOQUEADAS"));
	CLAVES_INICIALES_BLOQUEADAS = config_get_array_value(config, "CLAVES_INICIALES_BLOQUEADAS");

	config_destroy(config);
}


t_Esi* creo_esi(t_respuesta_para_planificador respuesta,int32_t fd_esi){
	t_Esi* esi = malloc(sizeof(t_Esi));
	esi->estimacionRafagaAnterior = 0;
	esi->tiempoEnListo = 0;
	esi->lineaALeer = 0;
	esi->cantSentenciasProcesadas = 0;
	esi->status = 2;
	esi->fd = fd_esi;  //lo necesito para luego saber a quien mandar el send
	esi->id = respuesta.id_esi;

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
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
		list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
		list_remove(LIST_READY, 0);
	} else {
		sContinuarComunicacion = false;
	}
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
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
		ordeno_listas();
		t_Esi* unEsiListo = list_get(LIST_READY, 0);
		list_add(LIST_EXECUTE, unEsiListo);
		list_remove(LIST_READY, 0);
	}
	//pregunto si hay alguien al menos
	if (list_is_empty(LIST_EXECUTE) && list_is_empty(LIST_READY)) {
		sContinuarComunicacion = false;
	}
	return sContinuarComunicacion;

}

bool aplico_algoritmo(char clave[40]){
	//Aca estoy si recibi una respuesta de alguna tarea pedida al ESI (TAREAS QUE PUEDE SER REALIZADAS OK o BLOCKEADO
	//al ESI)

	//Incides a actualizar :
	/* contador Inicial (?)
	 *  Contador Real
	 *  Tiempo en listo
	 *  cantidad sentencias Porcesadas
	 * */

	pthread_mutex_lock(&MUTEX);//sirve para delimitar mi RC como atomica ya que usamos la misma variable entre hilos
	while (PLANIFICADOR_EN_PAUSA){
		//pthread_mutex_unlock(&MUTEX);
		pthread_cond_wait(&CONDICION_PAUSA_PLANIFICADOR,&MUTEX);
		//pthread_mutex_lock(&MUTEX);
	}
	pthread_mutex_unlock(&MUTEX);

	bool sContinuarComunicacion = true;
	//Pregunto si tengo alguno en EXECUTE (si esta vacio entra el primero de ready)
//		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
//			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
//			list_remove(LIST_READY, 0);
//		} else {
	t_Esi* esiEjecutando = list_get(LIST_EXECUTE, 0);
	if(esiEjecutando != NULL){
	esiEjecutando ->cantSentenciasProcesadas++;}
	ActualizarIndicesEnLista();
		//controlo si tiene el flag de bloqueado para mandarlo a la list_block
		if(bloqueado_flag() ==  1){
			//Aca tengo que actualizar la estimacion inicial anterior.
			//saco de EXECUTE a BLOQUEADO
			// sumo una sentencia mas procesada que seria el get
			t_Esi* esi= list_get(LIST_EXECUTE, 0);
			char* clave_block = malloc(sizeof(char)*40);
			strcpy(clave_block,clave);
			clave_block[strlen(clave_block)] = '\0';
			t_nodoBloqueado* esi_bloqueado = get_nodo_bloqueado(esi,clave_block);
			list_add(LIST_BLOCKED,esi_bloqueado);

			printf("Muevo de EJECUCION a BLOQUEADO al ESI ID:%d por la clave:%s\n",esi->id,clave_block);
			free(clave_block);


			//Caso donde ESI se bloqueo al hacer lo que le pedi
			desbloquea_flag(); //limpio el flag
			//Solo lo saco de EXEC (cuando supe que era bloqueado porque el coordinador me informo puse flag = 1 y copie de exec ->  bloqueado)
			list_remove(LIST_EXECUTE, 0);
			//TODO:ACTUALIZO CONTADORES
			ordeno_listas();
			//toma el primero de listo -> exec y lo saca de listo

			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
			//Blanqueo el Esi que pasa a ejecutando
			BlanquearIndices();
		}else{

			//caso donde ESI hizo lo que le pidieron OK, no esta bloqueado pero el algoritmo es con desalojo
			//si entra aca significa que hizo la operacion, osea podemos contar++ ;)
			if (strcmp(ALGORITMO_PLANIFICACION, "SJFD") == 0) {
				//Revisar , si la estimacion del primero es mayor al que actualmente tengo, no tengo que desalojar
				if(get_time_SJF(list_get(LIST_EXECUTE, 0)) < get_time_SJF(list_get(LIST_READY, 0))){
					IncrementarLinealeer(list_get(LIST_EXECUTE, 0));
					return sContinuarComunicacion;
				}
				//exc -> listo
				list_add(LIST_READY, list_get(LIST_EXECUTE, 0));
				list_remove(LIST_EXECUTE, 0);
				//TODO:ACTUALIZO CONTADORES
				//ordeno lista
				ordeno_listas();
				//el primero de listo va a exec
				list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
				list_remove(LIST_READY, 0);
				//Blanqueo el Esi que pasa a ejecutando
				BlanquearIndices();
			}else{
				//ACA estoy si el ESI hizo lo que le pedi OK sin bloquearse y tampoco hay desalojo
				//(si no esta bloqueado y no es con desalojo no hago nada, solo continuo la comunicacion con el,
				//no hace falta ordenar las lista ya q estas se ordenaran cuando se bloquee el ESI o cuando TERMINE)
				IncrementarLinealeer(list_get(LIST_EXECUTE, 0));
			}
		}
	return sContinuarComunicacion;

}

// Pongo en 0 el tiempo en listos del esi que voy a ejecutar.
void BlanquearIndices(){
	t_Esi* esiEjecutando = list_get(LIST_EXECUTE, 0);
		if(esiEjecutando != NULL){
		esiEjecutando ->tiempoEnListo = 0;}

}
void ActualizarIndices(t_Esi *esi){
  esi->tiempoEnListo ++;

}
void ActualizarIndicesEnLista(){
	// recorro la lista de ready y aplico funcion actualizar Indices
	list_iterate(LIST_READY,(void*)ActualizarIndices );
	ordeno_listas();
}
void IncrementarLinealeer(t_Esi *esi){
	esi->lineaALeer++;
}

void desbloquea_flag(){
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	un_esi->status = 0;
}

bool bloqueado_flag(){
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	return (un_esi->status == 1);
}

void ordeno_listas(){
	if (strcmp(ALGORITMO_PLANIFICACION, "SJF") == 0) {
		order_list(LIST_READY, (void*) ordenar_por_SJFt);
	}
	// Ordena igual que SJFD pero desaloja el ESI que actualmente este procesando.
	if (strcmp(ALGORITMO_PLANIFICACION, "SJFD") == 0) {
		order_list(LIST_READY, (void*) ordenar_por_SJFt);

	}
	if (strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0) {
		order_list(LIST_READY, (void*) ordenar_por_HRRN);
	}
}

//Envia permiso de hacer una lectura a ESI
/*
 * flags_continuar = 1 OK
 *
 * */
void continuar_comunicacion(){


	t_Esi * primer_esi = list_get(LIST_EXECUTE,0);
	if(primer_esi == NULL){
		list_remove(LIST_EXECUTE, 0);
	}else{
		if (send(primer_esi->fd, &(primer_esi->lineaALeer), sizeof(int32_t), 0) == -1) {
			printf("Error al tratar de enviar el permiso a ESI\n");
		}else{
			printf("Envie permiso de ejecucion linea: %d al ESI de ID: %d\n",primer_esi->lineaALeer, primer_esi->id);
		}
	}


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
	list_remove_and_destroy_by_condition(LIST_READY,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(LIST_EXECUTE,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(LIST_FINISHED,(void*) _esElfd, free);

	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}
	list_remove_and_destroy_by_condition(LIST_BLOCKED,(void*) _esElfdBlocked,(void*) free_nodoBLoqueado);

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);


}


//Buscara al esi que hizo ctrl+c y lo manda a terminado ()
void remove_esi_by_fd_finished(int32_t fd){
	bool _esElfd(t_Esi* un_esi) { return un_esi->fd == fd;}

	if(list_find(LIST_READY, (void*)_esElfd) != NULL){
		t_Esi* esi_terminado = list_find(LIST_READY, (void*)_esElfd);
		//le resto uno ya que al ingresar a listo se le sumo uno y
		//ahora estaba ejecutando pero no hizo nada, termino de una pork no tenia nada mas para ejecutar, entonces le restamos eso asi
		//queda bien guardado en la lisa de finalizado
		esi_terminado->lineaALeer--;
		list_add(LIST_FINISHED,esi_terminado);
		list_remove_by_condition(LIST_READY,(void*) _esElfd);
	}

	if(list_find(LIST_EXECUTE, (void*)_esElfd) != NULL){
		list_add(LIST_FINISHED,list_find(LIST_EXECUTE, (void*)_esElfd));
		list_remove_by_condition(LIST_EXECUTE,(void*) _esElfd);
	}


	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}

	if(list_find(LIST_BLOCKED, (void*)_esElfdBlocked) != NULL){
		t_nodoBloqueado * nodoBuscado = list_find(LIST_BLOCKED, (void*)_esElfdBlocked);
		list_add(LIST_FINISHED,nodoBuscado->esi);
		list_remove_by_condition(LIST_BLOCKED,(void*) _esElfdBlocked);
	}

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}

	if(list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador) != NULL){
		t_esiBloqueador * esiBloqueador = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		list_add(LIST_FINISHED,esiBloqueador->esi);
		list_remove_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador);
	}



}

//libero tooodos los get clave que tenia tomado el esi de fd
void free_recurso(int32_t fd){

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	int32_t cant_esis_borrar = 0;

	if(list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador) != NULL){
		cant_esis_borrar = list_count_satisfying(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
	}
	int32_t contador = 0;
	while (contador < cant_esis_borrar){

		t_esiBloqueador * eb = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		printf("Libero clave:%s de ESI ID:%d\n", eb->clave,eb->esi->id);
		//paso de bloqueado a listo a todos los ESIs que esperaban esa clave
		move_esi_from_bloqueado_to_listo(eb->clave);
		list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);
		contador++;
	}

}

//paso de bloqueado a listo (EL PRIMER) los esis que pedian esa clave
void move_esi_from_bloqueado_to_listo(char* clave){

	//No se ordena la listas de listos porque se re ordena cuando actualizo los incides.
	bool _esElid(t_nodoBloqueado* nodoBloqueado) { return (strcmp(nodoBloqueado->clave,clave)==0);}
	int32_t cant_esis_mover = 0;

	if(list_find(LIST_BLOCKED, (void*)_esElid) != NULL){
		cant_esis_mover = list_count_satisfying(LIST_BLOCKED, (void*)_esElid);
	}
	if(cant_esis_mover>0){
		t_nodoBloqueado* nodoBloqueado = list_find(LIST_BLOCKED,(void*) _esElid);
		list_remove_by_condition(LIST_BLOCKED,(void*) _esElid);
		t_Esi* esi = nodoBloqueado->esi;
		list_add(LIST_READY,esi);
		printf("Desbloqueo al ESI ID:%d ya que esperaba la clave: %s\n", esi->id,nodoBloqueado->clave);
	}else{
		printf("No hay ningun ESI para desbloquear por la clave: %s\n",clave);
	}

//	int32_t contador = 0;
//
//	while (contador < cant_esis_mover){
//		t_nodoBloqueado* nodoBloqueado = list_find(LIST_BLOCKED,(void*) _esElid);
//		list_remove_by_condition(LIST_BLOCKED,(void*) _esElid);
//		t_Esi* esi = nodoBloqueado->esi;
//		list_add(LIST_READY,esi);
//		contador++;
//		printf("Desbloqueo al ESI ID:%d ya que esperaba la clave: %s\n", esi->id,nodoBloqueado->clave);
//	}

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

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double  get_time_SJF(t_Esi* esi){
	// alpha lo leo por config = 5
	// cada vez que proceso el esi, le voy sumando los tiempos;
	if( esi == NULL){
		return 0;
	}
		if(esi->cantSentenciasProcesadas == 0){
			return ESTIMACION_INICIAL;
		}
		double result = (esi->cantSentenciasProcesadas * ALPHA) + ((1 - ALPHA)* esi->estimacionRafagaAnterior);
		esi->estimacionRafagaAnterior = result;
		return result;
		// estimacionRafagaAnterior seria tn , cada vez que desalojo tengo que actualizar el
		// contadorReal seria tn +1

}
// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. OK
double getT_time_HRRN(t_Esi* esi){
	if( esi == NULL){
			return 0;
	}
	if(esi->cantSentenciasProcesadas == 0){
		return ESTIMACION_INICIAL;
	}
	double result =  ( esi->cantSentenciasProcesadas +  get_time_SJF(esi) ) / get_time_SJF(esi);
	esi->estimacionRafagaAnterior = result;
	return result;
}

/*------------------------------ORDENAMIENTO SIN DESALOJO--------------------------*/
//Ordena mi list (seria sobre mi lista de ready)
void order_list(t_list* lista, void * funcion){
	list_sort(lista, (void*) funcion);
}

//Criterio de ordenamiento por get_time_SJF (sin desalojo)
//condicion para q el primer parametro este antes del segundo parametro
bool ordenar_por_SJFt(t_Esi * esi_menor, t_Esi * esi) {
	return (get_time_SJF(esi_menor) < get_time_SJF(esi));
}

////Criterio de ordenamiento por GetTimeTHR
////condicion para q el primer parametro este antes del segundo parametro
bool ordenar_por_HRRN(t_Esi * esi_menor, t_Esi * esi) {
	return (getT_time_HRRN(esi_menor) > getT_time_HRRN(esi));
}

void agregar_en_bloqueados(t_Esi *esi, char* clave){
	t_nodoBloqueado* nodoBloqueado = get_nodo_bloqueado(esi,clave);
	list_add(LIST_BLOCKED,nodoBloqueado);
}

//Tanto para lista de listos como para la de finalizados.
void agregar_en_Lista(t_list* lista, t_Esi *esi){
	esi->lineaALeer++;
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
	t_Esi* esi_buscado = list_find(LIST_EXECUTE,(void*) _esElid);

	list_remove_by_condition(LIST_EXECUTE,(void*) _esElid);
	agregar_en_Lista(LIST_FINISHED,esi_buscado);
}

void inicializo_semaforos(){
	pthread_mutex_init(&MUTEX,NULL);
	pthread_cond_init(&CONDICION_PAUSA_PLANIFICADOR, NULL);

}

