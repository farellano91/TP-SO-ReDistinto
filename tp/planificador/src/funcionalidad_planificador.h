/*
 * funcionalidad_planificador.h
 *
 *  Created on: 18 abr. 2018
 *      Author: utnso
 */

#ifndef FUNCIONALIDAD_PLANIFICADOR_H_
#define FUNCIONALIDAD_PLANIFICADOR_H_

#include <commons/collections/list.h>
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
#include <commons/string.h>

void desbloquea_flag();

//consulta si el flag de bloqueo esta activo, esto para saber si tengo q mandarlo a bloqueado
//cuando recibo su mensaje
bool bloqueado_flag();

//se usa durante la comunicacion con los esis
int PUERTO_ESCUCHA;

char* ALGORITMO_PLANIFICACION;
//double ALPHA;
#define ALPHA 0.5
int ESTIMACION_INICIAL;

char** CLAVES_INICIALES_BLOQUEADAS;

//se usa para hablar con el coordinador
char* IP_CONFIG_COORDINADOR;

int PUERTO_CONFIG_COORDINADOR;

//este flag es para el caso de que la consola me deje en pausa
bool PLANIFICADOR_EN_PAUSA;

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();


enum t_operationCode {
	GET = 0, SET = 1, STORE = 2,
};

// revisar los tipos si son correctos la info y si van aca .
typedef struct {
	int id;
	int fd;
	int status; //1:bloqueado 2:ok
	int contadorInicial;
	int contadorReal;
	int tiempoEnListo;
	int cantSentenciasProcesadas;

} t_Esi;

//Ojo: no olvidar reservar memoria cuando creemos un t_instruccion*
//typedef struct {
//	enum t_operationCode operation;
//	char clave[40];
//	char value[100];
//
//} t_instruccion;

//Ojo: no olvidar reservar memoria cuando creemos un t_nodoBloqueado*
typedef struct {
	t_Esi* esi;
	char* clave;

} t_nodoBloqueado;


typedef struct {
	t_Esi* esi;
	char* clave;

} t_esiBloqueador;


/*ESI envia al planificador una respuesta al saludo o a la instruccion que hizo
 * id_tipo_respuesta = 1 respuesta al saludo, lo cual solo lleva de datos el id_esi y el mensaje
 * id_tipo_respuesta = 2 respuesta una instruccion realizad, lo cual trae todo lleno
 * */
typedef struct Respuesta_para_planificador{
	int id_tipo_respuesta;
	int id_esi; //1 o 2
	char mensaje[100];
} __attribute__((packed)) t_respuesta_para_planificador;


t_list* LIST_READY;

//lista de t_nodoBloqueado
t_list* LIST_BLOCKED;

t_list* LIST_FINISHED;

t_list* LIST_EXECUTE;

//lista (tabla) de t_esi_bloqueador
t_list* LIST_ESI_BLOQUEADOR;

t_list* create_list();

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, char* clave);
t_esiBloqueador* get_esi_bloqueador(t_Esi* esi, char* clave);

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double  get_time_SJF(t_Esi* esi);

// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. VER
double getT_time_HRRN(t_Esi* esi);

// Inserto en la lista Finalizadas y lista de Listos.

// Inserto en la lista de bloqueas, pero en base al nodo block.

bool ordenar_por_SJFt(t_Esi * esi_menor, t_Esi * esi);

bool ordenar_por_HRRN(t_Esi * esi_menor, t_Esi * esi);

void agregar_en_Lista(t_list* lista, t_Esi *esi);

void agregar_en_bloqueados(t_Esi *esi, char* clave);

bool aplico_algoritmo_ultimo();

bool aplico_algoritmo_primer_ingreso();

bool aplico_algoritmo();

//Remueve (y libera) cualquiere t_Esi de todas las lista
void remove_esi_by_fd(int fd);

t_Esi* creo_esi(t_respuesta_para_planificador respuesta , int fd_esi);

void  continuar_comunicacion();

void order_list(t_list* lista, void * funcion);

void cambio_de_lista(t_list* list_desde,t_list* list_hasta, int id_esi);

void free_recurso(int i);

void free_claves_iniciales();
#endif /* FUNCIONALIDAD_PLANIFICADOR_H_ */
