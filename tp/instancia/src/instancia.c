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
	int sockfd = conectar_coodinador();
	saludo_inicial_coordinador(sockfd);

	//REcibo datos de entrada
	recibo_datos_entrada(sockfd);

	//TODO: leer los archivos .txt creador a partir del dump para asi
	//poder cargar mis estructuras administrativas

	//TODO: hilo para cada X cant de tiempo hacer DUMP

	//Envio mis datos (TODO:tendriamos que mandar tambien el tamaño libre q tengo si esk ya tenia algo guardado en mi .txt)
	envio_datos(sockfd);

	//recibo mensaje de si me pudieron agregar a la lista  o no
	recibo_mensaje_aceptacion(sockfd);

	while(1){
		int resultado = recibo_sentencia(sockfd);
		envio_resultado_al_coordinador(sockfd,resultado);//(TODO:deberia mandar mi espacio libre actualizado)
	}
	//Por ahora libero la memoria que me quedo (solo hasta agregar funcionalidad posta)
	free_algo_punt_nom();
	return EXIT_SUCCESS;
}
