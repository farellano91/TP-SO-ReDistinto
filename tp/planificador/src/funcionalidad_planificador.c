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

	ALPHA = config_get_double_value(config,"ALPHA");

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

bool aplico_algoritmo(){
//1.- Ordena la lista list_ready (VG)
if(!planificador_en_pausa){
	bool sContinuarComunicacion = true;
	if (strcmp(algoritmo_planificacion,"SJF") == 0){
		order_list(list_ready,  (void*) ordenar_por_SJFt);
	}
	// Ordena igual que SJFD pero desaloja el ESI que actualmente este procesando.
	if (strcmp(algoritmo_planificacion,"SJFD") == 0){
		 order_list(list_ready,  (void*) ordenar_por_SJFt);
			sContinuarComunicacion = false;
     }
	if (strcmp(algoritmo_planificacion,"HRRN") == 0){
		order_list(list_ready,  (void*) ordenar_por_HRRN);
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
	t_Esi * primer_esi = list_get(list_ready,0);
	if (send(primer_esi->fd, &flags_continuar, sizeof(int32_t), 0) == -1) {
		printf("Error al tratar de enviar el permiso a ESI");
	}
	printf("Envie permiso ESI de ID: %d\n", primer_esi->id);
}

// en base a este ejemplo: https://github.com/sisoputnfrba/ansisop-panel/blob/master/panel/kernel.c
void remove_esi_by_fd(t_list* list, int fd){
	bool _esElfd(int* _fd) { return *_fd == fd;}
	list_remove_and_destroy_by_condition(list,(void*) _esElfd, free);
}


t_list* create_list_ready(){
	t_list * Lready = list_create();

	return Lready;
}
t_list* create_list_blocked(){
	// oJO Ver bien como definimos la lista de bloqueados.
	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
	t_list * Lblocked = list_create();
	return Lblocked;
}
t_list* create_list_finished(){
	// oJO Ver bien como definimos la lista de bloqueados.
	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
	t_list* Lfinished = list_create();
	return Lfinished;
}

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, t_instruccion* instruccion){

	t_nodoBloqueado* nodoBloqueado = malloc(sizeof(t_nodoBloqueado));
	nodoBloqueado->esi = malloc(sizeof(t_Esi));
	nodoBloqueado->intruccion = malloc(sizeof(t_instruccion));


	nodoBloqueado->esi = esi;
	nodoBloqueado->intruccion = instruccion;

	return nodoBloqueado;
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

void agregar_en_bloqueados(t_Esi *esi, t_instruccion * instruccionBloqueante){
	t_nodoBloqueado* nodoBloqueado = get_nodo_bloqueado(esi,instruccionBloqueante);
	list_add( list_blocked,nodoBloqueado);
}

//Tanto para lista de listos como para la de finalizados.
void agregar_en_Lista(t_list* lista, t_Esi *esi){
		list_add( lista ,esi);
}
//TODO: criterio de ordenamiento SJF con DESALOJO

