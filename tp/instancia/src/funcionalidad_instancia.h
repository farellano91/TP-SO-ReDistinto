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
#include <arpa/inet.h>
#include <commons/config.h>
#include <commons/collections/list.h>

char* ip_config_coordinador;
int puerto_config_coordinador;
char* algoritmo_reemplazo;
char* punto_montaje;
char* nombre_instancia;
int intervalo_dump;


//Cargo los parametros desde el archivo config y los libero conforme deje de usarlos
void get_parametros_config();

void free_parametros_config();

//Recibo los datos para mis entradas (para armar el storage)
void recibo_datos_entrada(sockfd);

//Envio mis datos al coordinador
void envio_datos(sockfd);


int tamanio_entrada;

int cant_entrada;

//Para tener la tabla de entradas
t_list* tabla_entrada;

typedef struct {
	int clave;
	int numero_entrada;
	int tamanio_variable;
} t_fila_tabla_entrada;

#endif /* FUNCIONALIDAD_INSTANCIA_H_ */
