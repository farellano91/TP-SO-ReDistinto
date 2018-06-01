/*
 * funciones_planificador.h
 *
 *  Created on: 15/4/2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "funcionalidad_planificador.h"

void levantar_consola();
bool es_el_mismo_esi(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador);
bool tiene_lo_que_quiere(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador);
t_list* buscar_deadlock(t_list* bloqueadores,t_list* bloqueados);

#endif /* CONSOLA_H_ */
