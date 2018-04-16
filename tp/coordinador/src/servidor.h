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

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

#define MYIP "127.0.0.1"

void levantar_servidor_coordinador();
void enviar_saludo(int fdCliente);
int recibir_saludo(int fdCliente);

void atender_cliente(void* idSocketCliente);

//Para ESI
void recibo_lineas(int fdCliente);


#endif

