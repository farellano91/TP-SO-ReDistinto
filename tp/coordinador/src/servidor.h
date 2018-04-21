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
#include "funcionalidad_coordinador.h"

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

#define MYIP "127.0.0.1"

void levantar_servidor_coordinador();
void enviar_saludo(int fdCliente);
int recibir_saludo(int fdCliente);

void atender_cliente(void* idSocketCliente);

//Para ESI
void recibo_lineas(int fdCliente);

//Para planificador
int fd_planificador;

typedef struct InfoCoordinador{
	int id; //1 o 2
	char clave[40];
} __attribute__((packed)) t_InfoCoordinador;


#endif

