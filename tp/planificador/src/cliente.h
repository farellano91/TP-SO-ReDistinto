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


/*Protocolo:
 * Caso: donde el coordinador me envia una operacion (get o store) + clave
 * 1 : GET
 * 2 : STORE
 * CLAVE NO MAXIMO DE 40 CARACTERES (segun enunciado pag.19)
 * */
void recibirInfoCoordinador();

//Busca dentro de LIST_ESI_BLOQUEADOR si hay un esi de id y clave que ya tomo el recurso
bool find_recurso_by_clave_id(char* clave,int id_esi);

//libero el recurso (borro de lis_esi_bloqueador el esi q corresponda)
void libero_recurso_by_clave_id(char* clave,int id_esi);

//paso de bloqueado a listo todos los ESIs que querian esa clave
void move_all_esi_bloqueado_listo(char* clave);

void send_mensaje(int fdCoordinador,int tipo_respuesta);
#endif


