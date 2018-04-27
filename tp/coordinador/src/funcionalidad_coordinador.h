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
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>

pthread_mutex_t mutex;

pthread_cond_t CONDICION_LIBERO_PLANIFICADOR;


t_log * logger;

int puerto_escucha_conexion;
char* algoritmo_distribucion;
int32_t cantidad_entradas;
int32_t tamanio_entrada;
int RETARDO;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();

void configure_logger();

typedef struct {
	int fd;
	char* nombre_instancia;
	int tamanio_libre;
} t_Instancia;

t_list* LIST_INSTANCIAS;

t_list* create_list();

void envio_datos_entrada(int fd_instancia);

t_Instancia* creo_instancia(int fd_instancia);

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva);

void inicializo_semaforos();

#endif /* FUNCIONALIDAD_COORDINADOR_H_ */
