/*
 ============================================================================
 Name        : c.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "cliente.h"
#include "consola.h"
#include "servidor.h"
#include "planificador.h"
#include <commons/collections/list.h>

int main(void) {

	//PARA CONVERSAR CON EL COORDINADOR
	pthread_t punteroHiloInfoCoordinador;
	pthread_create(&punteroHiloInfoCoordinador, NULL,
			(void*) recibirInfoCoordinador, NULL);

	//PARA LEVANTAR LA CONSOLA
	pthread_t punteroHiloConsola;
	pthread_create(&punteroHiloConsola, NULL, (void*) levantar_consola, NULL);

	//PARA CONVERSAR CON EL/LOS ESI's
	pthread_t punteroHiloServidor;
	pthread_create(&punteroHiloServidor, NULL, (void*) levantar_servidor_planificador, NULL);

	pthread_join(punteroHiloInfoCoordinador, NULL);
	pthread_join(punteroHiloConsola, NULL);
	return EXIT_SUCCESS;
}

