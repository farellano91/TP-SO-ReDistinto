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
	strcpy(algoritmo_planificacion,config_get_string_value(config, "ALGORITMO_PLANIFICACION"));

	estimacion_inicial = config_get_int_value(config,"ESTIMACION_INICIAL");

	ip_config_coordinador = malloc(sizeof(char) * 100);
	strcpy(ip_config_coordinador,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));

	puerto_config_coordinador = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	claves_iniciales_bloqueadas = malloc(sizeof(char) * 100);
	strcpy(claves_iniciales_bloqueadas,config_get_string_value(config, "CLAVES_INICIALES_BLOQUEADAS"));

	config_destroy(config);
}

//
//
//t_list createlistReady(){
//	t_list * Lready = list_create();
//	return Lready;
//}
//t_list createlistBlocked(){
//	// oJO Ver bien como definimos la lista de bloqueados.
//	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
//	t_list * Lblocked = list_create();
//	return Lblocked;
//}
//t_list createlistFinished(){
//	// oJO Ver bien como definimos la lista de bloqueados.
//	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
//	t_list * Lfinished = list_create();
//	return Lfinished;
//}
//
//t_nodoBloqueado GetNodoBloqueado(t_Esi esi, t_instruccion instruccion){
//	t_nodoBloqueado nodoBloqueado ;
//	nodoBloqueado->esi = esi;
//	nodoBloqueado->intruccion = instruccion;
//
//	return nodoBloqueado;
//}
//
//// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
//double  GetTimeSJF(t_Esi esi){
//	// alpha lo leo por config = 5
//	// cada vez que proceso el esi, le voy sumando los tiempos;
//int alpha = 5;
//
//return (esi->contadorInicial * alpha) + ((1 - alpha)* esi->contadorReal);
//
//}
//
//
//// el cantidad de sentencias procesadas
//// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. VER
//double GetTimeTHR(t_Esi esi, int cantSentenciasProcesadas){
//
//return ( cantSentenciasProcesadas +  GetTimeSJF(esi) ) / GetTimeSJF(esi);
//}
