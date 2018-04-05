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
#include <openssl/md5.h> // Para calcular el MD5

void levantarServidor();

#define MYPORT 8087    // Puerto al que conectarán los usuarios

#define BACKLOG 10     // Cuántas conexiones pendientes se mantienen en cola

//#define MYIP "192.168.0.12"
#define MYIP "127.0.0.1"

typedef struct DatosCliente {
		   char*  ipCliente;
		   int   idSocketCliente;
} TipoDatosCliente;

pthread_mutex_t mutex;


// A continuacion estan las estructuras con las que nos vamos a manejar.
typedef struct  {
  int id_mensaje;
  int legajo;
  char nombre[40];
  char apellido [40];
} __attribute__((packed)) Alumno;

typedef struct {
  int id;
  int len;
} __attribute__((packed)) ContentHeader;

#endif


