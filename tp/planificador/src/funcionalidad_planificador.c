#include "funcionalidad_planificador.h"


void free_parametros_config(){
	free(algoritmo_planificacion);
	free(ip_config_coordinador);
	free(claves_iniciales_bloqueadas);
}

void get_parametros_config() {
	t_config* config = config_create("config.cfg");
	if (!config) {
		printf("No encuentro el archivo config\n");
		//MUERO
		exit(1);
	}

	puerto_escucha = config_get_int_value(config,"PUERTO_ESCUCHA");

	algoritmo_planificacion = malloc(sizeof(char) * 100);
	// HRRN , SJF, SJFD
	strcpy(algoritmo_planificacion,config_get_string_value(config, "ALGORITMO_PLANIFICACION"));

	estimacion_inicial = config_get_int_value(config,"ESTIMACION_INICIAL");

	ip_config_coordinador = malloc(sizeof(char) * 100);
	strcpy(ip_config_coordinador,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	puerto_config_coordinador = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	claves_iniciales_bloqueadas = malloc(sizeof(char) * 100);
	strcpy(claves_iniciales_bloqueadas,config_get_string_value(config, "CLAVES_INICIALES_BLOQUEADAS"));

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
//1.- Ordena la lista list_ready (VG)
	bool con_desalojo = false;
	if (!planificador_en_pausa) {
		bool sContinuarComunicacion = true;
		if (strcmp(algoritmo_planificacion, "SJF") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
		}
		// Ordena igual que SJFD pero desaloja el ESI que actualmente este procesando.
		if (strcmp(algoritmo_planificacion, "SJFD") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
			con_desalojo = false;
		}
		if (strcmp(algoritmo_planificacion, "HRRN") == 0) {
			order_list(list_ready, (void*) ordenar_por_HRRN);
		}

		if (list_is_empty(list_execute) && !list_is_empty(list_ready)) {
			list_add(list_execute, list_get(list_ready, 0));
			list_remove(list_ready, 0);
		} else {
			if(con_desalojo){
				//el primero de ready lo mando a execute (si es que hay)
				//el q estaba en execute pasa a ready (si es que hay)
			}
		}

		//pregunto si hay alguien al menos
		if (list_is_empty(list_execute) && list_is_empty(list_ready)) {
			sContinuarComunicacion = false;
		}
		return sContinuarComunicacion;
	}
	return false;
}

bool aplico_algoritmo(){
//1.- Ordena la lista list_ready (VG)
	bool con_desalojo = false;
	if (!planificador_en_pausa) {
		bool sContinuarComunicacion = true;
		if (strcmp(algoritmo_planificacion, "SJF") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
		}
		// Ordena igual que SJFD pero desaloja el ESI que actualmente este procesando.
		if (strcmp(algoritmo_planificacion, "SJFD") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
			con_desalojo = true;
		}
		if (strcmp(algoritmo_planificacion, "HRRN") == 0) {
			order_list(list_ready, (void*) ordenar_por_HRRN);
		}

		//Pregunto si tengo alguno en EXECUTE (si esta vacio entra el primero de ready)
		if (list_is_empty(list_execute) && !list_is_empty(list_ready)) {
			list_add(list_execute, list_get(list_ready, 0));
			list_remove(list_ready, 0);
		} else {
			if(con_desalojo){
				//el primero de ready lo mando a execute
				//el q estaba en execute pasa a ready
			}
		}
		return sContinuarComunicacion;
	}
	return false;
}

bool aplico_algoritmo_primer_ingreso(){
//1.- Ordena la lista list_ready (VG)
	bool con_desalojo = false;

	if (!planificador_en_pausa) {
		bool sContinuarComunicacion = true;
		if (strcmp(algoritmo_planificacion, "SJF") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
		}
		// Ordena igual que SJFD pero desaloja el ESI que actualmente este procesando.
		if (strcmp(algoritmo_planificacion, "SJFD") == 0) {
			order_list(list_ready, (void*) ordenar_por_SJFt);
			con_desalojo = true;
		}
		if (strcmp(algoritmo_planificacion, "HRRN") == 0) {
			order_list(list_ready, (void*) ordenar_por_HRRN);
		}

		//Pregunto si tengo alguno en EXECUTE (si esta vacio entro)
		if (list_is_empty(list_execute) && !list_is_empty(list_ready)) {
			list_add(list_execute, list_get(list_ready, 0));
			list_remove(list_ready, 0);
		} else {
			if(con_desalojo){
				//el primero de ready lo mando a execute
				//el q estaba en execute pasa a ready
			}else{
				sContinuarComunicacion = false;
			}

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
	t_Esi * primer_esi = list_get(list_execute,0);
	if (send(primer_esi->fd, &flags_continuar, sizeof(int32_t), 0) == -1) {
		printf("Error al tratar de enviar el permiso a ESI\n");
	}else{
		printf("Envie permiso de ejecucion al ESI de ID: %d\n", primer_esi->id);
	}

}

// en base a este ejemplo: https://github.com/sisoputnfrba/ansisop-panel/blob/master/panel/kernel.c

void free_nodoBLoqueado(t_nodoBloqueado* nodoBloqueado){
	free(nodoBloqueado->esi);
	free(nodoBloqueado);

}
void remove_esi_by_fd(int fd){
	bool _esElfd(t_Esi* un_esi) { return un_esi->fd == fd;}
	list_remove_and_destroy_by_condition(list_ready,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(list_execute,(void*) _esElfd, free);
	list_remove_and_destroy_by_condition(list_finished,(void*) _esElfd, free);

	bool _esElfdBlocked(t_nodoBloqueado* nodo_bloqueado) { return nodo_bloqueado->esi->fd == fd;}
	list_remove_and_destroy_by_condition(list_blocked,(void*) _esElfd,(void*) free_nodoBLoqueado);

}

void free_recurso(int fd){
	//libero la clave que tenia tomado este esi (si es que tenia algo)
	bool _esElfdEsiBloqueador(t_esiBloqueador* esi_bloqueador) { return esi_bloqueador->esi->fd == fd;}
	list_remove_and_destroy_by_condition(list_esi_bloqueador,(void*) _esElfdEsiBloqueador,(void*) free_nodoBLoqueado);

}

t_list* create_list(){
	t_list * Lready = list_create();

	return Lready;
}

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, char clave[40]){

	t_nodoBloqueado* nodoBloqueado = malloc(sizeof(t_nodoBloqueado));
	nodoBloqueado->esi = malloc(sizeof(t_Esi));

	nodoBloqueado->esi = esi;
	strcpy(nodoBloqueado->clave,clave);

	return nodoBloqueado;
}

//creo t_esiBloqueador usando un esi (que no deberia liberarlo aun) y su clave
t_esiBloqueador* get_esi_bloqueador(t_Esi* esi, char clave[40]){

	t_esiBloqueador* esiBloqueador = malloc(sizeof(t_esiBloqueador));
	esiBloqueador->esi = malloc(sizeof(t_Esi));

	esiBloqueador->esi = esi;
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

void agregar_en_bloqueados(t_Esi *esi, char clave[40]){
	t_nodoBloqueado* nodoBloqueado = get_nodo_bloqueado(esi,clave);
	list_add(list_blocked,nodoBloqueado);
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

