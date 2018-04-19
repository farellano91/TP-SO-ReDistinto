#ifndef CLIENTEH
#define CLIENTEH

#include <pthread.h>
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
#include "funcionalidad_planificador.h"

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

int conectar_coodinador();
void saludo_inicial_coordinador(int sockfd);

typedef struct InfoCoordinador{
	int id; //1 o 2
	char clave[40];
} __attribute__((packed)) t_InfoCoordinador;

/*Protocolo:
 * Caso: donde el coordinador me envia una operacion (get o store) + clave
 * 1 : GET
 * 2 : STORE
 * CLAVE NO MAXIMO DE 40 CARACTERES (segun enunciado pag.19)
 * */
void recibirInfoCoordinador();

#endif


