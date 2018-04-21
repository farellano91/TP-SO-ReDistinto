
#include "funcionalidad_planificador.h"
#include "cliente.h"
#include "consola.h"
#include "servidor.h"


int main(void) {

	get_parametros_config();
	planificador_en_pausa = false;
	//PARA CONVERSAR CON EL COORDINADOR
	pthread_t punteroHiloInfoCoordinador;
	pthread_create(&punteroHiloInfoCoordinador, NULL,(void*) recibirInfoCoordinador, NULL);

	//PARA LEVANTAR LA CONSOLA
	pthread_t punteroHiloConsola;
	pthread_create(&punteroHiloConsola, NULL, (void*) levantar_consola, NULL);

	//PARA CONVERSAR CON EL/LOS ESI's
	pthread_t punteroHiloServidor;
	pthread_create(&punteroHiloServidor, NULL, (void*) levantar_servidor_planificador, NULL);

	pthread_join(punteroHiloInfoCoordinador, NULL);
	pthread_join(punteroHiloConsola, NULL);
	pthread_join(punteroHiloServidor, NULL);

	return EXIT_SUCCESS;
}

