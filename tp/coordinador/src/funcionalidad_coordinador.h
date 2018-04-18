/*
 * funcionalidades_coordinador.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONALIDAD_COORDINADOR_H_
#define FUNCIONALIDAD_COORDINADOR_H_

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

int puerto_escucha_conexion;
char* algoritmo_distribucion;
int cantidad_entradas;
int tamanio_entrada;
int retardo;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();


#endif /* FUNCIONALIDAD_COORDINADOR_H_ */
