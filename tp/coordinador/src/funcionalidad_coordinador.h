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
#include <commons/string.h>

pthread_mutex_t MUTEX;

pthread_cond_t CONDICION_LIBERO_PLANIFICADOR;


//Para planificador
int FD_PLANIFICADOR;

t_log * LOGGER;

int PUERTO_ESCUCHA_CONEXION;
char* ALGORITMO_DISTRIBUCION;
int32_t CANTIDAD_ENTRADAS;
int32_t TAMANIO_ENTRADA;
int RETARDO;

typedef struct {
	int fd;
	char* nombre_instancia;
	int tamanio_libre;
} t_Instancia;

int envio_tarea_instancia(int32_t id_operacion, t_Instancia * instancia, int32_t id_esi);

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();

void configure_logger();

t_list* LIST_INSTANCIAS;

t_list* create_list();

void envio_datos_entrada(int fd_instancia);

t_Instancia* creo_instancia(int fd_instancia);

void agrego_instancia_lista(t_list* list,t_Instancia* instancia_nueva);

void inicializo_semaforos();

int aplicarAlgoritmoDisctribucion(char *);

void envio_tarea_planificador(int32_t id_operacion,char* clave_recibida,int32_t id_esi);

void loggeo_info(int32_t id_operacion,int32_t id_esi,char* clave_recibida,char* valor_recibida);

int reciboRespuestaInstancia(t_Instancia * instancia);

#endif /* FUNCIONALIDAD_COORDINADOR_H_ */
