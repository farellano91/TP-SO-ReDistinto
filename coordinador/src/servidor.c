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
				printf("Error de Comunicación: Se fue ESI de fd:%d!!!\n", fd_esi);
				break;
			}

		} else {
			printf("Proceso sentencia de ESI ID:%d\n",id_esi);

			int32_t operacion = 0;
			if ((numbytes = recv(fd_esi, &operacion, sizeof(int32_t), 0))
					<= 0) {
				if (numbytes == 0) {
					//si el cliente se fue
					printf("Error de Comunicación: Se fue ESI de fd:%d!!!\n", fd_esi);
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
						loggeo_info(GET, id_esi, clave, "");
						if(excede_tamanio(clave)){
							loggeo_respuesta("GET",id_esi,ABORTA_ESI_ERROR_TAMANIIO_CLAVE);
							free(clave);
							break;
						}

						resultado_linea = envio_tarea_planificador(GET,clave,id_esi);
						if(resultado_linea != OK_BLOQUEADO_PLANIFICADOR){
							//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS, si no esta la registro sin instancia
							if(!exist_clave_registro_instancias(clave)){
								printf("La clave no existe en el sistema, la creamos...\n");
								t_registro_instancia* registro_nuevo = creo_registro_instancia("",clave);
								pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
								list_add(LIST_REGISTRO_INSTANCIAS,registro_nuevo);
								pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
							}
						}

						//loggeo respuesta
						loggeo_respuesta("GET",id_esi,resultado_linea);
						free(clave);
						break;
					case SET:
						clave_valor = get_clave_valor(fd_esi); //noc porque si metemos get_clave_valor dentro de aplicoA.. no recibe los valores posta
						loggeo_info(SET, id_esi, clave_valor[0],clave_valor[1]);

						if(excede_tamanio(clave_valor[0])){
							loggeo_respuesta("SET",id_esi,ABORTA_ESI_ERROR_TAMANIIO_CLAVE);
							free(clave_valor[0]);
							free(clave_valor[1]);
							free(clave_valor);
							break;
						}

						//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS
						if(exist_clave_registro_instancias(clave_valor[0])){
							printf("Existe la clave en el sistema\n");
							//ENVIAR MENSAJE AL PLANIFICADOR PARA DESCARTAR ERROR CLAVE NO BLOQUEADA SOLO ESO, NO HACE NADA UN SET DESDE EL COOR AL PLANI
							int32_t respuesta = envio_tarea_planificador(SET,clave_valor[0],id_esi);

							if(respuesta == ABORTA_ESI_CLAVE_NO_BLOQUEADA){
								resultado_linea = ABORTA_ESI_CLAVE_NO_BLOQUEADA;
							}else{
								pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
								bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave_valor[0])== 0;}
								t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);
								pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);

								if(strcmp(registro_instancia->nombre_instancia,"")==0){
									//No hay ninguna instancia con esta clave
									t_Instancia * instancia = busco_instancia_por_algortimo(clave_valor[0],0);
									if(instancia == NULL){
										//error al tratar de elegir una instancia
										resultado_linea = FALLA_ELEGIR_INSTANCIA;
									}else{
										resultado_linea = envio_tarea_instancia(SET,instancia,clave_valor);
									}
								}else{
									//Existe una instancia con esa clave asignada
									resultado_linea = envio_tarea_instancia(SET,get_instancia_by_name(registro_instancia->nombre_instancia),clave_valor);
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
						loggeo_info(STORE, id_esi, clave,"");

						if(excede_tamanio(clave)){
							loggeo_respuesta("STORE",id_esi,ABORTA_ESI_ERROR_TAMANIIO_CLAVE);
							free(clave);
							break;
						}

						//ANALIZA EXISTENCIA DE CLAVE EN LIST_REGISTRO_INSTANCIAS
						if(exist_clave_registro_instancias(clave)){
							printf("Existe la clave en el sistema\n");

							//ENVIAR MENSAJE AL PLANIFICADOR PARA DESCARTAR ERROR CLAVE NO BLOQUEADA SOLO ESO, NO HACE NADA UN SET DESDE EL COOR AL PLANI
							int32_t respuesta = envio_tarea_planificador(SET,clave,id_esi);
							if(respuesta == ABORTA_ESI_CLAVE_NO_BLOQUEADA){
								resultado_linea = ABORTA_ESI_CLAVE_NO_BLOQUEADA;
								loggeo_respuesta("STORE",id_esi,resultado_linea);
							}
							//--------------------------------------------------------

							else{
								printf("Planificador informa clave si bloqueada, opero sin problema\n");
								pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
								bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave)== 0;}
								t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);
								pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
								if(strcmp(registro_instancia->nombre_instancia,"")==0){
									//No hay ninguna instancia con esta clave
									printf("No existe ninguna instancia que tenga esta clave\n");
									resultado_linea = FALLA_SIN_INSTANCIA_CLAVE_STORE;
									loggeo_respuesta("STORE",id_esi,resultado_linea);
								}else{
									//Existe una instancia con esa clave asignada
									printf("Existe una instancia en instancia-clave que tiene esa clave, la uso\n");
									pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
									bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave)== 0;}
									t_registro_instancia * registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);
									pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);
									resultado_instancia = envio_recibo_tarea_store_instancia(STORE,clave,get_instancia_by_name(registro_instancia->nombre_instancia));
									loggeo_respuesta("STORE",id_esi,resultado_instancia);

									//si la instancia lo pudo hacer, recien le pido al planificador q lo haga
									if(resultado_instancia == OK_STORE_INSTANCIA){
										resultado_planificador = envio_tarea_planificador(STORE,clave,id_esi);
										loggeo_respuesta("STORE",id_esi,resultado_planificador);
									}

									//Juntamos ambas respuestas para dar una sola al esi
									if(resultado_planificador == OK_PLANIFICADOR
											&& (resultado_instancia == OK_SET_INSTANCIA || resultado_instancia == OK_STORE_INSTANCIA)){
										resultado_linea = OK_PLANIFICADOR;
									}else{resultado_linea = FALLO_PLANIFICADOR;}
								}
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


//esto es para limitar solo a 39-> letras + 1-> \0
bool excede_tamanio(char* clave){
	return ((strlen(clave) + 1) > 39 );
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
	int32_t resultado_planificador = -1;
	int numbytes = 0;
	if ((numbytes = recv(FD_PLANIFICADOR, &resultado_planificador, sizeof(int32_t), 0)) <= 0) {
		if(numbytes < 0){
			printf("ERROR: al tratar de recibir respuesta del planificador\n");
		}else{
			printf("El planificador se desconecto!\n");
		}
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
		while(1){
			int32_t respuesta = recibo_resultado_planificador();
			if(respuesta == -1){
				//se desconecto el planificador
				exit(1);//muero!!
			}else{
				pthread_mutex_lock(&MUTEX_RESPUESTA_PLANIFICADOR);
				RESPUESTA_PLANIFICADOR = respuesta;
				pthread_mutex_unlock(&MUTEX_RESPUESTA_PLANIFICADOR);
				pthread_cond_signal(&CONDICION_RESPUESTA_PLANIFICADOR);
			}
		}

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
				int32_t espacio_libre = reciboEspacioLibre(fdCliente);
				int32_t respuesta = reciboRespuestaInstancia(fdCliente);
				if(respuesta != OK_STATUS){
					//guardamos solo si no existe, si esta, lo actualizo
					cargo_instancia_respuesta(instancia_nueva->nombre_instancia,respuesta,espacio_libre);

					if(respuesta == ABORTA_ESI_CLAVE_INNACCESIBLE){
						remove_instancia(fdCliente);//Si se desconecto limpio la list_instancia
						pthread_cond_signal(&CONDICION_RECV_INSTANCIA);
						break; //para salir del while y que se vaya
					}
					pthread_cond_signal(&CONDICION_RECV_INSTANCIA);
				}else{
					//si es status, me enviara la longitud y valor de la clave
					cargo_respuesta_status(fdCliente);
					pthread_cond_signal(&CONDICION_RESPUESTA_STATUS);
				}

			}
			break;
		}
	}

	//FIN
	close(fdCliente);
	free((int *) idSocketCliente);

}

//si es status, me enviara la longitud y valor de la clave
void cargo_respuesta_status(int fd_instancia){
	int32_t long_clave = 0;
	int numbytes = 0;
	if ((numbytes = recv(fd_instancia, &long_clave, sizeof(int32_t), 0)) <= 0) {
		printf("No se pudo recibir le tamaño del valor de la clave para status\n");
		close(fd_instancia);
	}else{
		char* clave_recibida = malloc(long_clave);
		if ((numbytes = recv(fd_instancia, clave_recibida, long_clave, 0)) <= 0) {
			printf("No se pudo recibir el valor de la clave para status\n");
			free(clave_recibida);
			close(fd_instancia);

		}else{
			pthread_mutex_lock(&MUTEX_RESPUESTA_STATUS);
			free(RESPUESTA_STATUS);
			RESPUESTA_STATUS = malloc(long_clave);
			strcpy(RESPUESTA_STATUS,clave_recibida);
			RESPUESTA_STATUS[long_clave - 1 ] = '\0';
			pthread_mutex_unlock(&MUTEX_RESPUESTA_STATUS);
			free(clave_recibida);
		}
	}
}


void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		free_parametros_config();
		log_destroy(LOGGER);
		destruyo_semaforos();
		exit(dummy);
	}
}

void levantar_servidor_coordinador() {

	RESPUESTA_PLANIFICADOR = 0;

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
	my_addr.sin_addr.s_addr = inet_addr(IP_CONFIG_MIO); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
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
	destruyo_semaforos();

}

void destruyo_semaforos(){
	pthread_mutex_destroy(&MUTEX_RECV_INSTANCIA);
	pthread_cond_destroy(&CONDICION_RECV_INSTANCIA);

	pthread_mutex_destroy(&MUTEX_INSTANCIA);
	pthread_cond_destroy(&CONDICION_INSTANCIA);

	pthread_mutex_destroy(&MUTEX_REGISTRO_INSTANCIA);
	pthread_cond_destroy(&CONDICION_REGISTRO_INSTANCIA);

	pthread_mutex_destroy(&MUTEX_INDEX);

	pthread_mutex_destroy(&MUTEX_RESPUESTA_STATUS);
	pthread_cond_destroy(&CONDICION_RESPUESTA_STATUS);

	pthread_mutex_destroy(&MUTEX_RESPUESTA_PLANIFICADOR);
	pthread_cond_destroy(&CONDICION_RESPUESTA_PLANIFICADOR);
}

//Que no tenga exit(1)
void levantar_servidor_status(){
	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;

	RESPUESTA_STATUS = malloc(sizeof(char)*2);
	strcpy(RESPUESTA_STATUS,"");
	RESPUESTA_STATUS[1]='\0';

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error al abrir el socket de escucha\n");

		exit(1);
	}
	printf("Se creo el socket status correctamente\n");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		printf("Address already in use\n");

		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(PUERTO_ESCUCHA_CONEXION_STATUS);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(IP_CONFIG_MIO); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf("Fallo el bind\n");

		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {

		printf("Fallo el listen\n");
		exit(1);
	}
	printf("Socket status escuchando!!!\n");

	//-------
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {

		exit(1);
	}
	//--------
	sin_size = sizeof(struct sockaddr_in);

	//4° acepta y atiende al planificador cuando pida STATUS
	int fdNuevo = 0;
	if ((fdNuevo = accept(sockfd,
			(struct sockaddr *) &their_addr, &sin_size)) == -1) {
		//printf("No se pudo conectar el planificador para manejar status\n");
	}
	while(1){
		//el planificador desde la consola me envia la longitud y clave para el status y espra un paquete con [valor de clave|nombre instancia]
		int32_t leng_clave = 0;
		int numbytes = 0;
		if ((numbytes = recv(fdNuevo, &leng_clave, sizeof(int32_t), 0)) <= 0 ) {
			//printf("No se pudo recibir el tamaño de la clave\n");
		}else{
			char* clave_status = malloc(leng_clave);
			if ((numbytes = recv(fdNuevo, clave_status,leng_clave, 0)) <= 0 ) {
				//printf("No se pudo recibir la clave\n");

			}else{
				if(clave_status != NULL){
				printf("Planificador pide status clave: %s\n",clave_status);
				if(exist_clave_registro_instancias(clave_status)){
					t_registro_instancia * registro_instancia;
					//existe la clave en el sistema
					pthread_mutex_lock(&MUTEX_REGISTRO_INSTANCIA);
					bool _existRegistrInstancia(t_registro_instancia* reg_instancia) { return strcmp(reg_instancia->clave,clave_status)== 0;}
					registro_instancia = list_find(LIST_REGISTRO_INSTANCIAS, (void*)_existRegistrInstancia);
					pthread_mutex_unlock(&MUTEX_REGISTRO_INSTANCIA);

					if(strcmp(registro_instancia->nombre_instancia,"")==0){
						//No hay ninguna instancia con esta clave
						t_Instancia * instancia;
						instancia = busco_instancia_por_algortimo(clave_status,1);
						if(instancia == NULL){
							//no tengo ninguna futura instancia para mandarlo
							int32_t tipo = 5;
							if (send(fdNuevo, &tipo,sizeof(int32_t), 0) == -1) {

								printf("No se pudo enviar el resultado de status al planificador\n");

							}
							printf("La clave existe, pero no hay ninguna instancia futura que la pueda contener\n");
						}else{
							//uso el nombre de instancia
							int32_t tipo = 4;
							int32_t leng = strlen(instancia->nombre_instancia) +1 ;
							void* buffer = malloc(sizeof(int32_t)*2 + leng);
							memcpy(buffer,&tipo,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t),&leng,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t)*2,instancia->nombre_instancia,leng);
							if (send(fdNuevo, buffer,sizeof(int32_t)*2 + leng, 0) == -1) {

								printf("No se pudo enviar el resultado de status al planificador\n");

							}
							free(buffer);
							printf("La clave existe e iria a la instancia: %s\n",instancia->nombre_instancia);
						}
					}else{
						//Existe una instancia con esa clave asignada (tengo q mandar nombre de instancia y el valor tambien)
						char * valor = envio_recibo_pedido_valor(registro_instancia,clave_status);
						if(valor != NULL){
							int32_t tipo = 3;
							int32_t leng_instancia = strlen(registro_instancia->nombre_instancia) +1 ;
							int32_t leng_valor = strlen(valor) + 1;

							void* buffer = malloc(sizeof(int32_t)*3 + leng_instancia + leng_valor);
							memcpy(buffer,&tipo,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t),&leng_instancia,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t)*2,registro_instancia->nombre_instancia,leng_instancia);
							memcpy(buffer + sizeof(int32_t)*2 + leng_instancia,&leng_valor,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t)*3 + leng_instancia,valor,leng_valor);
							if (send(fdNuevo, buffer,sizeof(int32_t)*3 + leng_instancia + leng_valor, 0) == -1) {
								printf("No se pudo enviar el resultado de status al planificador\n");
							}
							free(buffer);
							printf("La clave existe, valor:%s en instancia:%s\n",valor,registro_instancia->nombre_instancia);
						}else{//existia una instancia pero esta desconectada caso:2
							int32_t tipo = 2;

							int32_t leng = strlen(registro_instancia->nombre_instancia) +1 ;
							void* buffer = malloc(sizeof(int32_t)*2 + leng);
							memcpy(buffer,&tipo,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t),&leng,sizeof(int32_t));
							memcpy(buffer + sizeof(int32_t)*2,registro_instancia->nombre_instancia,leng);
							if (send(fdNuevo, buffer,sizeof(int32_t)*2 + leng, 0) == -1) {

								printf("No se pudo enviar el resultado de status al planificador\n");

							}

							printf("La clave existe y esta dentro de uns instancia pero esta esta desconectada\n");
						}
						free(valor);
					}
				}else{
					//no existe en el sistema la clave
					int32_t tipo = 1;
					if (send(fdNuevo, &tipo,sizeof(int32_t), 0) == -1) {

						printf("No se pudo enviar el resultado de status al planificador\n");

					}
					printf("No existe en el sistema esa clave\n");

					}
				}
				free(clave_status);
				printf("Resultado de status enviado\n");
			}
		}
	}
	close(fdNuevo);
}


char * envio_recibo_pedido_valor(t_registro_instancia* reg_instancia,char* clave_status){
	int32_t long_clave = strlen(clave_status) + 1;
	int32_t id_operacion = STATUS;
	void* bufferEnvio = malloc(sizeof(int32_t) * 2 + long_clave);
	memcpy(bufferEnvio, &id_operacion, sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t), &long_clave, sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t)*2,clave_status,long_clave);

	pthread_mutex_lock(&MUTEX_INSTANCIA);
	bool _esInstanciaNombre(t_Instancia* instancia_buscada){
		return (strcmp(instancia_buscada->nombre_instancia,reg_instancia->nombre_instancia) == 0);
	}
	t_Instancia* instancia = list_find(LIST_INSTANCIAS,(void*)_esInstanciaNombre);
	pthread_mutex_unlock(&MUTEX_INSTANCIA);

	if(instancia == NULL){
		return NULL;
	}

	//limpio la respuesta
	pthread_mutex_lock(&MUTEX_RESPUESTA_STATUS);
	strcpy(RESPUESTA_STATUS,"");
	pthread_mutex_unlock(&MUTEX_RESPUESTA_STATUS);

	if (send(instancia->fd, bufferEnvio,sizeof(int32_t) * 2 + long_clave, 0) == -1) {
		free(bufferEnvio);
		return NULL;
	}

	printf("Se envio tarea status a la instancia para saber el valor de la clave\n");


	pthread_mutex_lock(&MUTEX_RESPUESTA_STATUS);
	while(strcmp(RESPUESTA_STATUS,"")==0){
		pthread_cond_wait(&CONDICION_RESPUESTA_STATUS,&MUTEX_RESPUESTA_STATUS);
	}
	pthread_mutex_unlock(&MUTEX_RESPUESTA_STATUS);


	pthread_mutex_lock(&MUTEX_RESPUESTA_STATUS);
	int32_t leng_respuesta = strlen(RESPUESTA_STATUS) + 1;
	char* respuesta = malloc(leng_respuesta);
	strcpy(respuesta,RESPUESTA_STATUS);
	respuesta[strlen(RESPUESTA_STATUS)] = '\0';

	strcpy(RESPUESTA_STATUS,"");
	pthread_mutex_unlock(&MUTEX_RESPUESTA_STATUS);

	free(bufferEnvio);
	return respuesta;
}

