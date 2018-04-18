/*
 * planificador.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>

// revisar los tipos si son correctos la info y si van aca .
typedef struct {
	int id;
	int status;
	int contadorInicial;
	int contadorReal;
	int tiempoEnListo;

} t_Esi	;

enum t_operationCode
{
GET = 0,
SET = 1,
STORE= 2,

};

typedef struct{
	enum t_operationCode operation;
	char * key;
	char * value;

} t_instruccion;

typedef struct{
t_Esi esi;
t_instruccion intruccion;

} t_nodoBloqueado;

t_list createlistReady(){
	t_list * Lready = list_create();
	return Lready;
}
t_list createlistBlocked(){
	// oJO Ver bien como definimos la lista de bloqueados.
	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
	t_list * Lblocked = list_create();
	return Lblocked;
}
t_list createlistFinished(){
	// oJO Ver bien como definimos la lista de bloqueados.
	// cada nodo de la lista de bloqueados contiene el esi bloqueado y la instruccion que lo bloqueo
	t_list * Lfinished = list_create();
	return Lfinished;
}

t_nodoBloqueado GetNodoBloqueado(t_Esi esi, t_instruccion instruccion){
	t_nodoBloqueado nodoBloqueado ;
	nodoBloqueado->esi = esi;
	nodoBloqueado->intruccion = instruccion;

	return nodoBloqueado;
}

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double  GetTimeSJF(t_Esi esi){
	// alpha lo leo por config = 5
	// cada vez que proceso el esi, le voy sumando los tiempos;
int alpha = 5;

return (esi->contadorInicial * alpha) + ((1 - alpha)* esi->contadorReal);

}


// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. VER
double GetTimeTHR(t_Esi esi, int cantSentenciasProcesadas){

return ( cantSentenciasProcesadas +  GetTimeSJF(esi) ) / GetTimeSJF(esi);
}

// Inserto en la lista Finalizadas y lista de Listos.

// Inserto en la lista de bloqueas, pero en base al nodo block.


#endif /* PLANIFICADOR_H_ */
