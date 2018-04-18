#ifndef FUNCIONALIDAD_ESI_H_
#define FUNCIONALIDAD_ESI_H_

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

char* ip_config_coordinador;
int puerto_config_coordinador;
char* ip_config_planificador;
int puerto_config_planificador;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();

#endif /* FUNCIONALIDAD_ESI_H_ */
