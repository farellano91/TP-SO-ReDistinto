#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_instancia.h"
#include "cliente.h"

int main(void) {

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	CANT_ENTRADA = 0;
	TAMANIO_ENTRADA = 0;
	get_parametros_config();
	//TODO: leer los archivos .txt creador a partir del dump
	//TODO: crea/cargar mis estructuras administrativas
	int sockfd = conectar_coodinador();
	saludo_inicial_coordinador(sockfd);

	//REcibo datos de entrada
	recibo_datos_entrada(sockfd);
	//Envio mis datos
	envio_datos(sockfd);

	while(1){
		int resultado = recibo_sentencia(sockfd);
		envio_resultado_al_coordinador(sockfd,resultado);
	}
	//Por ahora libero la memoria que me quedo (solo hasta agregar funcionalidad posta)
	free_algo_punt_nom();
	return EXIT_SUCCESS;
}
