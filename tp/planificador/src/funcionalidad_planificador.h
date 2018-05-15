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
#include <pthread.h>

pthread_mutex_t MUTEX;
pthread_cond_t CONDICION_PAUSA_PLANIFICADOR;

void desbloquea_flag();

//consulta si el flag de bloqueo esta activo, esto para saber si tengo q mandarlo a bloqueado
//cuando recibo su mensaje
bool bloqueado_flag();

//se usa durante la comunicacion con los esis
int32_t PUERTO_ESCUCHA;

char* ALGORITMO_PLANIFICACION;
//double ALPHA;
double ALPHA;
int32_t ESTIMACION_INICIAL;

char** CLAVES_INICIALES_BLOQUEADAS;

//se usa para hablar con el coordinador
char* IP_CONFIG_COORDINADOR;

int32_t PUERTO_CONFIG_COORDINADOR;

//este flag es para el caso de que la consola me deje en pausa
bool PLANIFICADOR_EN_PAUSA;

void ActualizarIndicesEnLista();

void ActualizarIndices();

void IncrementarLinealeer();
//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

//libera todos los parametros que tenga
void free_parametros_config();

//muevo todo los esis que establan bloqueados a listo segun la clave
void move_esi_from_bloqueado_to_listo(char* clave);

enum t_operationCode {
	GET = 1, SET = 2, STORE = 3,
};

// revisar los tipos si son correctos la info y si van aca .
typedef struct {
	int32_t id;
	int32_t fd;
	int32_t status; //1:bloqueado 2:ok (este va a ser un flags que me servira para saber cuando tengo que mandar a bloqueado un esi cuando tenga su respuesta)
	int32_t tiempoProcesando;
	int32_t tiempoEnListo;
	int32_t cantSentenciasProcesadas;
	int32_t lineaALeer; //cada vez q le pido a un esi q haga algo, le estoy mandando un numero de linea a leer

} t_Esi;



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
 * id_tipo_respuesta = 3 respuesta de que ya no tiene nada mas que leer y termino feliz
 * */
typedef struct Respuesta_para_planificador{
	int32_t id_tipo_respuesta;
	int32_t id_esi; //1 o 2
	char mensaje[100];
	char clave[40];
} __attribute__((packed)) t_respuesta_para_planificador;


t_list* LIST_READY;

//lista de t_nodoBloqueado
t_list* LIST_BLOCKED;

t_list* LIST_FINISHED;

t_list* LIST_EXECUTE;

//lista (tabla) de t_esi_bloqueador
t_list* LIST_ESI_BLOQUEADOR;

t_list* create_list();

t_Esi * newEsi;

t_nodoBloqueado* get_nodo_bloqueado(t_Esi* esi, char* clave);
t_esiBloqueador* get_esi_bloqueador(t_Esi* esi, char* clave);

// dado un esi que me llega como parametro, me estima cuantas rafagas de cpu consumira.
double  get_time_SJF(t_Esi* esi);

// el cantidad de sentencias procesadas
// si lo pongo como un parametro del esi, voy a tener que recorrer nodo por nodo para ir acumulando. VER
double getT_time_HRRN(t_Esi* esi);


bool ordenar_por_tiempo(t_Esi * esi_menor, t_Esi * esi);


void agregar_en_Lista(t_list* lista, t_Esi *esi);

void agregar_en_bloqueados(t_Esi *esi, char* clave);

bool aplico_algoritmo_ultimo();

bool aplico_algoritmo_primer_ingreso();

void aplicarFormulaPlanificacion(t_Esi *esi);

bool aplico_algoritmo();

//Remueve (y libera) cualquiere t_Esi de todas las lista
void remove_esi_by_fd(int32_t fd);

//REmueve un esi de donde este y lo manda a finish (si estaba en una lo saca)
void remove_esi_by_fd_finished(int32_t fd);

t_Esi* creo_esi(t_respuesta_para_planificador respuesta , int32_t fd_esi);

void  continuar_comunicacion();

void order_list(t_list* lista, void * funcion);

void cambio_de_lista(t_list* list_desde,t_list* list_hasta, int32_t id_esi);

void cambio_ejecutando_a_finalizado(int32_t id_esi);

void free_recurso(int32_t i);

void free_claves_iniciales();

void ordeno_listas();

void inicializo_semaforos();

void BlanquearIndices();


#endif /* FUNCIONALIDAD_PLANIFICADOR_H_ */
