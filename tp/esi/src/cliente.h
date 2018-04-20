#ifndef CLIENTEH
#define CLIENTEH

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
#include "funcionalidad_esi.h"

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

int conectar_servidor(int puerto , char* ip, char* nombre);

void saludo_inicial_servidor(int fd,char* nombre);

/*ESI envia al planificador una respuesta al saludo o a la instruccion que hizo
 * id_tipo_respuesta = 1 respuesta al saludo, lo cual solo lleva de datos el id_esi y el mensaje
 * id_tipo_respuesta = 2 respuesta una instruccion realizad, lo cual trae todo lleno
 * id_tipo_respuesta = 3 respuesta indicado en mensaje que ya termine de leer todo
 * */
typedef struct Respuesta_para_planificador{
	int id_tipo_respuesta;
	int id_esi; //1 o 2
	char mensaje[100];
	char instruccion[40];
} __attribute__((packed)) t_respuesta_para_planificador;

#endif


