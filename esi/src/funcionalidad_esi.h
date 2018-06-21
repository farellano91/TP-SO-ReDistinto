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
#include <parsi/parser.h>

char* IP_CONFIG_COORDINADOR;

int PUERTO_CONFIG_COORDINADOR;

char* IP_CONFIG_PLANIFICADOR;

int PUERTO_CONFIG_PLANIFICADOR;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();

int32_t ID_ESI_OBTENIDO;

enum t_respuesta_de_coordinador {
	ABORTA = 1, //esto me mandaria a terminado
	OK = 2,
	OK_BLOQUEADO = 3,
	ABORTA_INNACCESIBLE = 4,

};

enum t_respuesta_para_planificador {
	ABORTA_PLANIFICADOR = 5, //esto me mandaria a terminado
	OK_PLANIFICADOR = 6,
	ABORTA_INNACCESIBLE_PLANIFICADOR = 7,
	NUEVO_PLANIFICADOR = 8,
};

#endif /* FUNCIONALIDAD_ESI_H_ */
