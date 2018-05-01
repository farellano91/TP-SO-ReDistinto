/*
 ============================================================================
 Name        : coordinador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_coordinador.h"
#include "servidor.h"

int main(void) {
	get_parametros_config();
	configure_logger();
	inicializo_semaforos();
	levantar_servidor_coordinador();
	return EXIT_SUCCESS;
}
