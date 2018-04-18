#include <stdio.h>
#include <stdlib.h>
#include "funcionalidad_instancia.h"
#include "cliente.h"

int main(void) {

	get_parametros_config();

	int sockfd = conectar_coodinador();
	saludo_inicial_coordinador(sockfd);

	//TODO:comunicarme con coordinador para crear mi void*
	//TODO:preparar funciones para guardado.txt


	//Por ahora libero la memoria que me quedo (solo hasta agregar funcionalidad posta)
	free(algoritmo_reemplazo);
	free(punto_montaje);
	free(nombre_instancia);
	//


	return EXIT_SUCCESS;
}
