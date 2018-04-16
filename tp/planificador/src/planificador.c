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

int main(void) {

	//PARA CONVERSAR CON EL COORDINADOR
	pthread_t punteroHiloInfoCoordinador;
	pthread_create(&punteroHiloInfoCoordinador, NULL, (void*) recibirInfoCoordinador, NULL);

	//PARA LEVANTAR LA CONSOLA
	pthread_t punteroHiloConsola;
	pthread_create(&punteroHiloConsola, NULL, (void*) levantar_consola, NULL);

	//TODO:PARA CONVERSAR CON EL ESI:levanto servidor, saludar al ESI, encolarlos y planificarlos
	//-------------------------------codigo------------------------------

	pthread_join(punteroHiloInfoCoordinador, NULL);
	pthread_join(punteroHiloConsola, NULL);
	return EXIT_SUCCESS;
}
