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

//Para planificador
int FD_PLANIFICADOR;

typedef struct InfoCoordinador{
	int id; //operacion GET=1 SET=2 STORE=3
	int id_esi;
	char* clave;
} t_InfoCoordinador;

char* get_valor_recibido(int fd_esi);

char* get_clave_recibida(int fd_esi);

void envio_resultado_esi(int fd_esi,int resultado_linea,int id_esi);

int recibo_resultado_planificador();

void envio_tarea_planificador(int32_t id_operacion,char* clave_recibida,int32_t id_esi);

void envio_tarea_instancia(int32_t id_operacion, char* clave_recibida, char* valor_recibido , int32_t id_esi);

void levantar_servidor_coordinador();

void enviar_saludo(int fdCliente);

int recibir_saludo(int fdCliente);

void atender_cliente(void* idSocketCliente);

//Para ESI
void recibo_lineas(int fdCliente);
/**/

#endif

