#ifndef FUNCIONALIDAD_INSTANCIA_H_
#define FUNCIONALIDAD_INSTANCIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <commons/config.h>
#include <sys/wait.h>
#include <signal.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

char* IP_CONFIG_COORDINADOR;
int PUERTO_CONFIG_COORDINADOR;
char* ALGORITMO_REEMPLAZO;
char* PUNTO_MONTAJE;
char* NOMBRE_INSTANCIA;
int INTERVALO_DUMP;

int TAMANIO_ENTRADA;

int CANT_ENTRADA;

//Para el storage
char** STORAGE;

//Para saber que entrada esta vacia o ocuapda y con x cant. de operaciones hechas
typedef struct {
	int libre; //1:esta libre 0:esta ocupado
	int cant_operaciones; //si esta libre es cero
	int tamanio_libre;
} t_registro_diccionario_entrada;

t_dictionary * DICCIONARITY_ENTRADA;

t_dictionary* create_diccionarity();

//Para tener la tabla de entradas
typedef struct {
	int numero_entrada;
	char* clave;
	int tamanio_valor;
} t_registro_tabla_entrada;

t_list* TABLA_ENTRADA;

t_list* create_list();


enum t_respuesta_al_coordinador {
	FALLO_OPERACION_INSTANCIA = 5, //fallo al hacer algo
	OK_SET_INSTANCIA = 6,
	OK_STORE_INSTANCIA = 7,
};

enum t_operacion {
	SET = 2,
	STORE = 3,
};

void intHandler(int dummy);

void free_algo_punt_nom();

void free_registro_tabla_entrada(t_registro_tabla_entrada* registro);

void free_registro_diccionario_entrada(t_registro_diccionario_entrada* registro);

void free_estruct_admin();

//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

void free_parametros_config();

//Recibo los datos para mis entradas (para armar el storage)
void recibo_datos_entrada(int sockfd);

int obtener_espacio_libre();

//Envio mis datos al coordinador
void envio_datos(int sockfd);

int recibo_sentencia(int fd_coordinador);

void envio_resultado_al_coordinador(int sockfd,int resultado);

void recibo_mensaje_aceptacion(int fd_coordinador);

void cargar_estructuras(char* clave,char* valor,int tamanio_contenido);

void reestablesco_archivo(char* nombre_archivo);

void reestablecer_datos();

void inicializo_estructuras();

t_registro_diccionario_entrada* get_new_registro_dic_entrada(int libre,int cant_operaciones,int tamanio_libre);

t_registro_tabla_entrada* get_new_registro_tabla_entrada(int numero_entrada,char* clave,int tamanio_valor);

void cargo_actualizo_tabla(char* clave,int numero_entrada,int tamanio_contenido);

void cargo_actualizo_diccionario(int numero_entrada,int tamanio_contenido);

#endif /* FUNCIONALIDAD_INSTANCIA_H_ */
