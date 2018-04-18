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

#endif


