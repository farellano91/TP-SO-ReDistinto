#include "servidor.h"


void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

int recibir_saludo(int fdCliente) {
	int resultado = 0;
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(fdCliente, &longitud, sizeof(int32_t), 0)) == -1) {
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(fdCliente, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir mensaje saludo\n");
		//MUERO
		exit(1);
	}
	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);

	if (strstr(mensajeSaludoRecibido, "ESI") != NULL) {
		resultado = 1;
	}
	if (strstr(mensajeSaludoRecibido, "PLA") != NULL) {
		resultado = 2;
	}
	if (strstr(mensajeSaludoRecibido, "INS") != NULL) {
		resultado = 3;
	}

	free(mensajeSaludoRecibido);
	return resultado;
}

/*PROTOCOLO para envio saludo al ESI
 * ESI <-> PLANIFICADOR
 * [INT + CHAR*]
 * INT: len del mensaje saludos
 * CHAR*: mensaje saludo
 * */

void enviar_saludo(int fdCliente) {

	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el COORDINADOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(fdCliente, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);


}

void recibo_lineas(int fd_esi) {
	int numbytes = 0;
	int32_t resultado_linea = 0;
	while (1) {
		sleep(RETARDO);
		int32_t id_esi = 0;
		if ((numbytes = recv(fd_esi, &id_esi, sizeof(int32_t), 0))
				<= 0) {
			if (numbytes == 0) {
				//si el cliente se fue
				printf("Se fue: ESI de fd:%d, chau gato!!!\n", fd_esi);
				break;
			}

		} else {
			printf("Proceso sentencia de ESI ID:%d\n",id_esi);

			int32_t operacion = 0;
			if ((numbytes = recv(fd_esi, &operacion, sizeof(int32_t), 0))
					<= 0) {
				if (numbytes == 0) {
					//si el cliente se fue
					printf("Se fue: ESI de fd:%d, chau gato!!!\n", fd_esi);
					break;
				}
			} else {
				char**clave_valor;
				char* clave;
				int resultado_instancia = 0;
				int resultado_planificador = 0;
				switch (operacion) {
					case GET:
						clave = get_clave_recibida(fd_esi);
						loggeo_info(1, id_esi, clave, "");

						//TODO: ANALIZAR EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS, sino aborta al esi resultado_linea = ABORTA_ESI
						envio_tarea_planificador(1,clave,id_esi);
						resultado_linea = recibo_resultado_planificador();

						//loggeo respuesta
						loggeo_respuesta("GET",id_esi,resultado_linea);
						free(clave);
						break;
					case SET:
						clave_valor = get_clave_valor(fd_esi); //noc porque si metemos get_clave_valor dentro de aplicoA.. no recibe los valores posta
						loggeo_info(2, id_esi, clave_valor[0],clave_valor[1]);

						//TODO: Busco en tabla, si no esta aplico algoritmo, si esta, uso esa instancia defrente
						resultado_linea = aplicarAlgoritmoDisctribucion(ALGORITMO_DISTRIBUCION,clave_valor);//free(clave_valor) esta aca adentro

						loggeo_respuesta("SET",id_esi,resultado_linea);
						break;
					case STORE:
						clave = get_clave_recibida(fd_esi);
						loggeo_info(3, id_esi, clave,"");

						//TODO: ANALIZAR EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS, sino aborta al esi resultado_linea = ABORTA_ESI
						//por ahora le doy la primera instancia que encuentro pero eso tiene q cambiar
						if(list_get(LIST_INSTANCIAS,0) != NULL){
							resultado_instancia = envio_recibo_tarea_store_instancia(3,clave,list_get(LIST_INSTANCIAS,0));
						}else{
							printf("No hay instancias para usar\n");
							resultado_instancia = FALLO_DESCONEXION_INSTANCIA;
						}
						//

						envio_tarea_planificador(3,clave,id_esi);
						resultado_planificador = recibo_resultado_planificador();

						//tanto la operacion del lado de instancia como del planificador no debe fallar para dar el OK
						if((resultado_instancia != FALLO_DESCONEXION_INSTANCIA)&& (resultado_planificador != FALLO_PLANIFICADOR)
								&& (resultado_instancia != FALLO_OPERACION_INSTANCIA)){
							resultado_linea = resultado_planificador; //Si nadie fallo uso el numero del planificador
						}else{
							resultado_linea = FALLO_OPERACION_INSTANCIA; //Tambien puede ser FALLO_PLANIFICADOR, lo importante es mandar n°1
						}
						free(clave);
						//loggeo respuesta
						loggeo_respuesta("STORE",id_esi,resultado_linea);
						break;

					default:
						break;
				}

				//al margen de lo q devuelva el planif o instna, el : resultado_linea solo es 1,2,3
				envio_resultado_esi(fd_esi,resultado_linea,id_esi);
			}
		}
	}
}

char* get_clave_recibida(int fd_esi){
	int leng_clave = 0;
	int numbytes = 0;
	if ((numbytes = recv(fd_esi, &leng_clave, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño de la clave\n");
		//MUERO
		exit(1);
	}
	char* clave = malloc(sizeof(char) * leng_clave);
	if ((numbytes = recv(fd_esi, clave, sizeof(char) * leng_clave, 0)) == -1) {
		printf("No se pudo recibir la clave\n");
		//MUERO
		exit(1);
	}
	printf("Recibi clave: %s de longitud: %d correctamente\n",clave,leng_clave);
	return clave;

}

void envio_resultado_esi(int fd_esi,int resultado_linea,int id_esi){
	if (send(fd_esi, &resultado_linea,sizeof(int32_t), 0) == -1) {
		printf("No se pudo enviar resultado al ESI\n");
		exit(1);
	}
	printf("Envie resultado al ESI de ID:%d!!!\n",id_esi);
}

int recibo_resultado_planificador(){
	int32_t resultado_planificador = 0;
	int numbytes = 0;
	if ((numbytes = recv(FD_PLANIFICADOR, &resultado_planificador, sizeof(int32_t), 0)) <= 0) {
		if(numbytes < 0){
			printf("ERROR: al tratar de recibir respuesta del planificador\n");
		}else{
			//TODO: aca debo liberar el WAIT
			printf("El planificador se desconecto!\n");
		}
		FD_PLANIFICADOR = -1;

		//ABORTA ESI
		resultado_planificador = FALLO_PLANIFICADOR;
		pthread_cond_signal(&CONDICION_LIBERO_PLANIFICADOR); //Libero ahora si al planificador
	}else{
		printf("Respuesta del planificador recibida\n");

	}

	return resultado_planificador;

}


/*PROTOCOLO:
 * tipo_cliente: 1 -> ESI
 * tipo_cliente: 2 -> PLANIFICADOR
 * tipo_cliente: 3 -> INSTANCIA
 * */
void atender_cliente(void* idSocketCliente) {

	//Saludo a todos los q se conectaron
	int fdCliente = ((int *) idSocketCliente)[0];

	enviar_saludo(fdCliente);
	int tipo_cliente = recibir_saludo(fdCliente);
	switch (tipo_cliente) {

	case ESI:
		recibo_lineas(fdCliente);
		break;
	case PLANIFICADOR:
		FD_PLANIFICADOR = fdCliente;
		pthread_cond_wait(&CONDICION_LIBERO_PLANIFICADOR, &MUTEX); //lo detengo aca hasta q no lo usea mas
//		exit(1); //mato al coordinador
		break;
	case INSTANCIA:
		//1:Envio datos de entrada
		envio_datos_entrada(fdCliente);
		//2:Recibo datos para crear la instancia
		t_Instancia* instancia_nueva = creo_instancia(fdCliente);

		//return true si ya existe
		if(controlo_existencia(instancia_nueva)){
			//envio mensaje de rechazo porque ya tenemos una instancia con ese nombre ISSUE#1050
			send_mensaje_rechazo(instancia_nueva);
			free_instancia(instancia_nueva);
			break; //para salir del case y cerrar la comunicacion
		}else{
			//3:Encolo la INSTANCIA
			agrego_instancia_lista(LIST_INSTANCIAS,instancia_nueva);
			while(1){
				int32_t respuesta = reciboRespuestaInstancia(fdCliente);
				if(respuesta == FALLO_DESCONEXION_INSTANCIA){ //es necesario tener el if aca para los casos de desconexion de instancias agenas no me afecte
					remove_instancia(fdCliente);//Si se desconecto limpio la list_instancia
					break; //para salir del while y que se vaya
				}

				RESULTADO_INSTANCIA_VG = respuesta;
				pthread_cond_signal(&CONDICION_RECV_INSTANCIA);
			}
			break;
		}
	}
	//FIN
	close(fdCliente);
	free((int *) idSocketCliente);

}

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		free_parametros_config();
		log_destroy(LOGGER);
		pthread_mutex_destroy(&MUTEX);
		pthread_cond_destroy(&CONDICION_LIBERO_PLANIFICADOR);
		pthread_cond_destroy(&CONDICION_RECV_INSTANCIA);
		exit(dummy);

	}
}
void levantar_servidor_coordinador() {

	RESULTADO_INSTANCIA_VG = -1;

	FD_PLANIFICADOR = -1;

	//creo mi lista de instancia
	LIST_INSTANCIAS = create_list();

	//creo mi tabla registro instancias
	LIST_REGISTRO_INSTANCIAS = create_list();

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error al abrir el socket de escucha\n");
		free_parametros_config();
		exit(1);
	}
	printf("Se creo el socket correctamente\n");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		printf("Address already in use\n");
		free_parametros_config();
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(PUERTO_ESCUCHA_CONEXION);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(MYIP); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf("Fallo el bind\n");
		free_parametros_config();
		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {
		free_parametros_config();
		printf("Fallo el listen\n");
		exit(1);
	}
	printf("Socket escuchando!!!\n");

	//-------
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	//--------
	sin_size = sizeof(struct sockaddr_in);

	//4° acepta y atiende a los ESI's, INSTANCIAS's que se conecte
	while (1) {
		int *idSocketCliente = (int *) malloc(sizeof(int32_t));
		idSocketCliente[0] = -1; //TODO: que pasa con esta variable con varios hilos ?
		if ((idSocketCliente[0] = accept(sockfd,
				(struct sockaddr *) &their_addr, &sin_size)) == -1) {
			perror("Error al usar accept");
		}

		//CREAMOS UN HILO PARA ATENDER A CUALQUIER CLIENTE
		pthread_t punteroHilo;
		pthread_create(&punteroHilo, NULL, (void*) atender_cliente,
				idSocketCliente);

	}
	free_parametros_config();
	log_destroy(LOGGER);
	close(sockfd);
	pthread_mutex_destroy(&MUTEX);
	pthread_cond_destroy(&CONDICION_LIBERO_PLANIFICADOR);
	pthread_cond_destroy(&CONDICION_RECV_INSTANCIA);
}


