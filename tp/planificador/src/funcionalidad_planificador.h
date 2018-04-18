/*
 * funcionalidad_planificador.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONALIDAD_PLANIFICADOR_H_
#define FUNCIONALIDAD_PLANIFICADOR_H_

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <commons/config.h>

//se usa durante la comunicacion con los esis
int puerto_escucha;

char* algoritmo_planificacion;

int estimacion_inicial;

char* claves_iniciales_bloqueadas;

//se usa para hablar con el coordinador
char* ip_config_coordinador;

int puerto_config_coordinador;


//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();



enum t_operationCode {
	GET = 0, SET = 1, STORE = 2,
};

// revisar los tipos si son correctos la info y si van aca .
typedef struct {
	int id;
	int status;
	int contadorInicial;
	int contadorReal;
	int tiempoEnListo;

} t_Esi;

typedef struct {
	enum t_operationCode operation;
	char * key;
	char * value;

} t_instruccion;

typedef struct {
	t_Esi esi;
	t_instruccion intruccion;

} t_nodoBloqueado;

t_list* createlistReady();

t_list* createlistBlocked();

t_list* createlistFinished();

t_nodoBloqueado GetNodoBloqueado(t_Esi esi, t_instruccion instruccion);

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double GetTimeSJF(t_Esi esi);

// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. VER
double GetTimeTHR(t_Esi esi, int cantSentenciasProcesadas);

// Inserto en la lista Finalizadas y lista de Listos.

// Inserto en la lista de bloqueas, pero en base al nodo block.


#endif /* FUNCIONALIDAD_PLANIFICADOR_H_ */
