#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_instancia.h"
#include "cliente.h"

int main(void) {

	cant_entrada = 0;
	tamanio_entrada = 0;
	get_parametros_config();
	//TODO: leer los archivos .txt creador a partir del dump
	//TODO: crea/cargar mis estructuras administrativas
	int sockfd = conectar_coodinador();
	saludo_inicial_coordinador(sockfd);

	//REcibo datos de entrada
	recibo_datos_entrada(sockfd);
	//Envio mis datos
	envio_datos(sockfd);

	// TODO:espero q el coordinador me de una tarea para hacer


	//Por ahora libero la memoria que me quedo (solo hasta agregar funcionalidad posta)
	free(algoritmo_reemplazo);
	free(punto_montaje);
	free(nombre_instancia);
	//


	return EXIT_SUCCESS;
}
