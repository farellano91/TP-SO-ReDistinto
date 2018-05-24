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
	int32_t resultado_linea_final = 0;
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
						//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS, si no esta la registro sin instancia
						if(!exist_clave_registro_instancias(clave)){
							printf("La clave no existe en el sistema, la creamos...\n");
							t_registro_instancia* registro_nuevo = creo_registro_instancia("",clave);
							list_add(LIST_REGISTRO_INSTANCIAS,registro_nuevo);
						}
						envio_tarea_planificador(1,clave,id_esi);
						resultado_linea = recibo_resultado_planificador();

						//loggeo respuesta
						loggeo_respuesta("GET",id_esi,resultado_linea);
						free(clave);
						break;
					case SET:
						clave_valor = get_clave_valor(fd_esi); //noc porque si metemos get_clave_valor dentro de aplicoA.. no recibe los valores posta
						loggeo_info(2, id_esi, clave_valor[0],clave_valor[1]);

						//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS
						if(exist_clave_registro_instancias(clave_valor[0])){
							printf("Existe la clave en el sistema\n");
							//ENVIAR MENSAJE AL PLANIFICADOR PARA DESCARTAR ERROR CLAVE NO BLOQUEADA
							envio_tarea_planificador(2,clave_valor[0],id_esi);
							int32_t respuesta = recibo_resultado_planificador();//tiene un recv, solo se puede usar una vez
							if(respuesta == ABORTA_ESI_CLAVE_NO_BLOQUEADA){
								resultado_linea = ABORTA_ESI_CLAVE_NO_BLOQUEADA;
							}else{
								bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave_valor[0])== 0;}
								t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);

								if(strcmp(registro_instancia->nombre_instancia,"")==0){
									//No hay ninguna instancia con esta clave
									t_Instancia * instancia = busco_instancia_por_algortimo(ALGORITMO_DISTRIBUCION,clave_valor);
									if(instancia == NULL){
										//error al tratar de elegir una instancia
										resultado_linea = FALLA_ELEGIR_INSTANCIA;
									}else{
										resultado_linea = envio_tarea_instancia(2,instancia,clave_valor);
									}
								}else{
									//Existe una instancia con esa clave asignada
									resultado_linea = envio_tarea_instancia(2,get_instancia_by_name(registro_instancia->nombre_instancia),clave_valor);
								}
							}
						}else{
							printf("No existe la clave en el sistema\n");
							resultado_linea = ABORTA_ESI_CLAVE_NO_IDENTIFICADA;
						}
						loggeo_respuesta("SET",id_esi,resultado_linea);
						free(clave_valor[0]);
						free(clave_valor[1]);
						free(clave_valor);
						break;
					case STORE:
						clave = get_clave_recibida(fd_esi);
						loggeo_info(3, id_esi, clave,"");

						//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS
						if(exist_clave_registro_instancias(clave)){
							printf("Existe la clave en el sistema\n");
							bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave)== 0;}
							t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);

							if(strcmp(registro_instancia->nombre_instancia,"")==0){
								//No hay ninguna instancia con esta clave
								printf("No existe ninguna instancia que tenga esta clave\n");
								resultado_linea = FALLA_SIN_INSTANCIA_CLAVE_STORE;
								loggeo_respuesta("STORE",id_esi,resultado_linea);
							}else{
								//Existe una instancia con esa clave asignada
								printf("Existe una instancia en instancia-clave que tiene esa clave, la uso\n");
								bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave)== 0;}
								t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);
								resultado_instancia = envio_recibo_tarea_store_instancia(3,clave,get_instancia_by_name(registro_instancia->nombre_instancia));
								loggeo_respuesta("STORE",id_esi,resultado_instancia);

								envio_tarea_planificador(3,clave,id_esi);
								resultado_planificador = recibo_resultado_planificador();//aca vendra la respues si es por abor clave no bloquead
								loggeo_respuesta("STORE",id_esi,resultado_planificador);

								//Juntamos ambas respuestas para dar una sola al esi
								if(resultado_planificador == OK_PLANIFICADOR
										&& (resultado_instancia == OK_SET_INSTANCIA || resultado_instancia == OK_STORE_INSTANCIA)){
									resultado_linea = OK_PLANIFICADOR;
								}else{resultado_linea = FALLO_PLANIFICADOR;}
							}

						}else{
							printf("No existe la clave en el sistema\n");
							resultado_linea = ABORTA_ESI_CLAVE_NO_IDENTIFICADA;
							loggeo_respuesta("STORE",id_esi,resultado_linea);

						}

						free(clave);
						break;

					default:
						break;
				}
				resultado_linea_final = aplicar_filtro_respuestas(resultado_linea); //resultado_linea solo es 1,2,3
				envio_resultado_esi(fd_esi,resultado_linea_final,id_esi);
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
			printf("El planificador se desconecto!\n");
		}
		FD_PLANIFICADOR = -1;

		resultado_planificador = FALLA_PLANIFICADOR_DESCONECTADO;
		pthread_cond_signal(&CONDICION_LIBERO_PLANIFICADOR); //Libero ahora si al planificador
	}else{
		printf("Respuesta del planificador recibida\n");
		if(resultado_planificador == ABORTA_ESI_CLAVE_NO_BLOQUEADA){
			printf("Planificador informa error de clave no bloqueada\n");
		}
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
				int32_t tamanio_libre = reciboTamanioLibre(fdCliente);
				int32_t respuesta = reciboRespuestaInstancia(fdCliente);
				//guardamos solo si no existe, si esta, lo actualizo
				cargo_instancia_respuesta(instancia_nueva->nombre_instancia,respuesta,tamanio_libre);

				if(respuesta == ABORTA_ESI_CLAVE_INNACCESIBLE){
					remove_instancia(fdCliente);//Si se desconecto limpio la list_instancia
					pthread_cond_signal(&CONDICION_RECV_INSTANCIA);
					break; //para salir del while y que se vaya
				}
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
		pthread_mutex_destroy(&MUTEX_RECV_INSTANCIA);
		pthread_cond_destroy(&CONDICION_LIBERO_PLANIFICADOR);
		pthread_cond_destroy(&CONDICION_RECV_INSTANCIA);
		exit(dummy);

	}
}
void levantar_servidor_coordinador() {

	FD_PLANIFICADOR = -1;

	//creo mi lista de instancia
	LIST_INSTANCIAS = create_list();

	//creo mi tabla registro instancias (INTANCIA-CLAVE)
	LIST_REGISTRO_INSTANCIAS = create_list();

	//creo mi tabla de instancia respuestas, respuestas dada por las instnacias
	LIST_INSTANCIA_RESPUESTA = create_list();

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
	pthread_mutex_destroy(&MUTEX_RECV_INSTANCIA);
	pthread_cond_destroy(&CONDICION_LIBERO_PLANIFICADOR);
	pthread_cond_destroy(&CONDICION_RECV_INSTANCIA);
}


