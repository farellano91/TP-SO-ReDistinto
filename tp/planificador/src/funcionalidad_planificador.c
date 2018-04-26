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


t_Esi* creo_esi(t_respuesta_para_planificador respuesta,int fd_esi){
	t_Esi* esi = malloc(sizeof(t_Esi));
	esi->contadorInicial = 0;
	esi->contadorReal = 0;
	esi->tiempoEnListo = 0;
	esi->cantSentenciasProcesadas = 0;
	esi->fd = fd_esi;  //lo necesito para luego saber a quien mandar el send
	esi->id = respuesta.id_esi;

	return esi;
}

bool aplico_algoritmo_ultimo(){
	//si entro aca no estoy en exc, estoy en finish
	if (!PLANIFICADOR_EN_PAUSA) {
		bool sContinuarComunicacion = true;
		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
		}
		//pregunto si hay alguien al menos
		if (list_is_empty(LIST_EXECUTE) && list_is_empty(LIST_READY)) {
			sContinuarComunicacion = false;
		}
		return sContinuarComunicacion;
	}
	return false;
}

bool aplico_algoritmo(){
	if (!PLANIFICADOR_EN_PAUSA) {
		bool sContinuarComunicacion = true;
		//Pregunto si tengo alguno en EXECUTE (si esta vacio entra el primero de ready)
		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
		} else {
			//controlo si tiene el flag de bloqueado para mandarlo a la list_block
			if(bloqueado_flag()){
				desbloquea_flag(); //limpio el flag
				//Solo lo saco de EXEC (cuando supe que era bloqueado puse flag = 1 y copie de exec ->  bloqueado)
				list_remove(LIST_EXECUTE, 0);
				//toma el primero de listo -> exec y lo saca de listo
				list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
				list_remove(LIST_READY, 0);
			}else{
				//si entra aca significa que hizo la operacion, osea podemos contar++ ;)
				if (strcmp(ALGORITMO_PLANIFICACION, "SJFD") == 0) {
					//exc -> listo
					list_add(LIST_READY, list_get(LIST_EXECUTE, 0));
					list_remove(LIST_EXECUTE, 0);
					//ordeno lista
					order_list(LIST_READY, (void*) ordenar_por_SJFt);
					//el primero de listo va a exec
					list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
					list_remove(LIST_READY, 0);
				}
			}
		}
		return sContinuarComunicacion;
	}
	return false;
}

void desbloquea_flag(){
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	un_esi->status = 0;
}

bool bloqueado_flag(){
	t_Esi * un_esi  = list_get(LIST_EXECUTE, 0);
	return (un_esi->status == 1);
}

bool aplico_algoritmo_primer_ingreso(){
//1.- Ordena la lista LIST_READY (VG)
	if (!PLANIFICADOR_EN_PAUSA) {
		bool sContinuarComunicacion = true;
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

		//Pregunto si tengo alguno en EXECUTE (si esta vacio entro)
		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE, list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
		} else {
			sContinuarComunicacion = false;
		}
		return sContinuarComunicacion;
	}
	return false;
}
//Envia permiso de hacer una lectura a ESI
/*
 * flags_continuar = 1 OK
 *
 * */
void continuar_comunicacion(){

	int32_t flags_continuar = 1;
	t_Esi * primer_esi = list_get(LIST_EXECUTE,0);
	if(primer_esi == NULL){
		list_remove(LIST_EXECUTE, 0);
	}else{
		if (send(primer_esi->fd, &flags_continuar, sizeof(int32_t), 0) == -1) {
			printf("Error al tratar de enviar el permiso a ESI\n");
		}else{
			printf("Envie permiso de ejecucion al ESI de ID: %d\n", primer_esi->id);
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

void remove_esi_by_fd(int fd){
	bool _esElfd(t_Esi* un_esi) { return un_esi->fd == fd;}
	list_remove_and_destroy_by_condition(LIST_READY,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(LIST_EXECUTE,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(LIST_FINISHED,(void*) _esElfd, free);

	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}
	list_remove_and_destroy_by_condition(LIST_BLOCKED,(void*) _esElfdBlocked,(void*) free_nodoBLoqueado);

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);


}
//libero tooodos los get clave que tenia tomado el esi de fd
void free_recurso(int fd){

	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	int cant_esis_borrar = 0;

	if(list_find(LIST_BLOCKED, (void*)_esElfdEsiBloqueador) != NULL){
		cant_esis_borrar = list_count_satisfying(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
	}
	int contador = 0;
	while (contador < cant_esis_borrar){

		t_esiBloqueador * eb = list_find(LIST_ESI_BLOQUEADOR, (void*)_esElfdEsiBloqueador);
		printf("Libero clave:%s de ESI ID:%d\n", eb->clave,eb->esi->id);
		list_remove_and_destroy_by_condition(LIST_ESI_BLOQUEADOR,(void*) _esElfdEsiBloqueador,(void*) free_esiBloqueador);
		contador++;
	}

}

t_list* create_list(){
	t_list * Lready = list_create();

	return Lready;
}

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, char* clave){

	t_nodoBloqueado* nodoBloqueado = malloc(sizeof(t_nodoBloqueado));
//	nodoBloqueado->esi = malloc(sizeof(t_Esi));
	int len = strlen(clave) + 1;
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
	int len = strlen(clave) + 1;
	esiBloqueador->clave = malloc(sizeof(char)* len);
	strcpy(esiBloqueador->clave,clave);

	return esiBloqueador;
}

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double  get_time_SJF(t_Esi* esi){
	// alpha lo leo por config = 5
	// cada vez que proceso el esi, le voy sumando los tiempos;
	return (esi->contadorInicial * ALPHA) + ((1 - ALPHA)* esi->contadorReal);

}


// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. OK
double getT_time_HRRN(t_Esi* esi){

	return ( esi->cantSentenciasProcesadas +  get_time_SJF(esi) ) / get_time_SJF(esi);
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
		list_add( lista ,esi);
}
//TODO: criterio de ordenamiento SJF con DESALOJO

//Mueve mi esi de una lista a otra
void cambio_de_lista(t_list* list_desde,t_list* list_hasta, int id_esi){

	bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
	t_Esi* esi_buscado = list_find(list_desde,(void*) _esElid);

	list_remove_by_condition(list_desde,(void*) _esElid);
	agregar_en_Lista(list_hasta,esi_buscado);



}

