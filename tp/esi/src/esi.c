/*
 ============================================================================
 Name        : c.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "archivo.h"
#include "cliente.h"

void exit_gracefully(int return_nr) {
	exit(return_nr);
}

int main(int argc, char** argv) {

	get_parametros_config();

	//Conecta como cliente al coordinador
	int fd_coordinador = conectar_servidor(puerto_config_coordinador,ip_config_coordinador,"COORDINADOR");
	saludo_inicial_servidor(fd_coordinador,"COORDINADOR");

	//Conecta como cliente al planificador
	int fd_planificador = conectar_servidor(puerto_config_planificador,ip_config_planificador,"PLANIFICADOR");
	saludo_inicial_servidor(fd_planificador,"PLANIFICADOR");



//2.- lee el archivo linea por linea
	if (argc > 2) {
		puts("Ingreso 2 o mas parametros");
		exit_gracefully(-1);
	}
	FILE* file = txt_open_file(argv[1], "r");
	if (file == NULL) {
		txt_close_file(file);
		puts("Error al abrir el archivo ");
		exit_gracefully(-1);
	}
	char* line = malloc(sizeof(char) * 500);
	while (fgets(line, 500, file)) {
		//3.- se queda esperando permiso para envia la linea "parseada" al coordinador
		// es decir recv(fd_planificador, ....)
		//envio linea
		int longitud = strlen(line) + 1;
		line[strlen(line)] = '\0';
		void* bufferEnvio = malloc(sizeof(int32_t) + longitud);
		memcpy(bufferEnvio, &longitud, sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t), line, longitud);

		if (send(fd_coordinador, bufferEnvio, sizeof(int32_t) + longitud, 0) == -1) {
			perror("recv");
			exit(1);
		}
		printf("Envie linea: %s\n", line);
		free(bufferEnvio);

	}
	free(line);
	txt_close_file(file);
	close(fd_coordinador);
	return EXIT_SUCCESS;
}
