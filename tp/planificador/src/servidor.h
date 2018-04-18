/*
 * servidor.h
 *
 *  Created on: 25/3/2018
 *      Author: utnso
 */
#ifndef SERVIDORH
#define SERVIDORH

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
#include "funcionalidad_planificador.h"

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

#define MYIP "127.0.0.2"

int fdmax;
fd_set master;   // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()


void levantar_servidor_planificador();

void enviar_saludo(int fdCliente);

void atender_cliente(void* idSocketCliente);

#endif

