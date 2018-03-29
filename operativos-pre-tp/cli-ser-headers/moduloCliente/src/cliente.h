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

#define PORT 8082 // puerto al que vamos a conectar

#define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

//#define IP "192.168.0.12"
#define IP "127.0.0.1"

void levantarCliente();

#endif


