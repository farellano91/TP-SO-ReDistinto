
#include "funcionalidad_planificador.h"
#include "cliente.h"
#include "consola.h"
#include "servidor.h"


int main(int argc, char *argv[]) {

	if(argc < 2){
		puts("Falta el archivo de config");
		return EXIT_FAILURE;
	}

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	get_parametros_config(argv[1]);
	PLANIFICADOR_EN_PAUSA = false;
	inicializo_semaforos();
	configure_logger();
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

	pthread_mutex_destroy(&MUTEX);
	pthread_cond_destroy(&CONDICION_PAUSA_PLANIFICADOR);

	return EXIT_SUCCESS;
}

