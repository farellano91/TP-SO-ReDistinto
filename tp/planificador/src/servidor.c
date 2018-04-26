#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

//int recibir_resultado(int fdCliente) {
//	int resultado = 0;
//	void* bufferRecibido = malloc(sizeof(char) * 29);
//	char * mensajeSaludoRecibido = malloc(sizeof(char) * 29);
//	int numbytes = 0;
//	if ((numbytes = recv(fdCliente, bufferRecibido, 29, 0)) <= 0) {
//		if (numbytes == 0) {
//			//si el cliente se fue
//			printf("Se fue: socket %d, chau gato!!!\n", fdCliente);
//			close(fdCliente); // si ya no conversare mas con el cliente, lo cierro
//
//		}
//
//	} else {
//		memcpy(mensajeSaludoRecibido, bufferRecibido, 29);
//		printf("Saludo recibido: %s\n", mensajeSaludoRecibido);
//	}
//
//	if (strstr(mensajeSaludoRecibido, "ESI") != NULL) {
//		resultado = 1;
//	}
//	free(bufferRecibido);
//	free(mensajeSaludoRecibido);
//	return resultado;
//}


/*PROTOCOLO para envio saludo al ESI
 * ESI <-> PLANIFICADOR
 * [INT + CHAR* + INT]
 * INT: len del mensaje saludos
 * CHAR*: mensaje saludo
 * INT: id_esi que me da el planificador (solo si id = 1)
 * */

//esto es solo para el ESI
void enviar_saludo(int fdCliente, int id_esi) {

	char * mensajeSaludoEnviado = malloc(sizeof(char) *100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el PLANIFICADOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;
	int32_t id_para_esi = id_esi;
	void* bufferEnvio = malloc(sizeof(int32_t)*2 + sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);
	memcpy(bufferEnvio + sizeof(int32_t) + longitud_mensaje,&id_para_esi,sizeof(int32_t));

	if (send(fdCliente, bufferEnvio,sizeof(int32_t)*2 + sizeof(char)*longitud_mensaje, 0) == -1) {
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);
}

void atender_esi(void* idSocketCliente) {

	int fdCliente = ((int *) idSocketCliente)[0];
	int id_esi = ((int *) idSocketCliente)[1];
	enviar_saludo(fdCliente,id_esi);
	free((int *) idSocketCliente);

}

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		exit(dummy);

	}
}

void cargo_claves_iniciales(){
	void cargo_en_list_esi_bloqueador(char* clave){
		t_Esi * un_esi = malloc(sizeof(t_Esi));
		//Esis de id = 0 son los que estan en claves bloqueadas INICIALES
		un_esi->id = 0;
		t_esiBloqueador* esiBLo = get_esi_bloqueador(un_esi,clave);
		list_add(LIST_ESI_BLOQUEADOR,esiBLo);
		printf("Se cargo la clave:%s bloqueada INICIALMENTE\n", clave);
	}
	string_iterate_lines(CLAVES_INICIALES_BLOQUEADAS,(void*)cargo_en_list_esi_bloqueador);
}

void crear_listas_globales(){
	LIST_READY = create_list();
	LIST_BLOCKED = create_list();
	LIST_FINISHED = create_list();
	LIST_EXECUTE = create_list();
	LIST_ESI_BLOQUEADOR = create_list();

}
void levantar_servidor_planificador() {
	//cree mis listas globales
	crear_listas_globales();

	//cargo las claves bloqueadas iniciales
	cargo_claves_iniciales();

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	int contador_id_esi = 0;
	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;
	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	int i;

	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error al abrir el socket de escucha\n");
		free(ALGORITMO_PLANIFICACION);
		free_claves_iniciales();
		//MUERE EL HILO
		exit(1);
	}
	printf("Se creo el socket correctamente\n");

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("address already in use");
		free(ALGORITMO_PLANIFICACION);
		free_claves_iniciales();
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(PUERTO_ESCUCHA);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(MYIP); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		printf("Fallo el bind\n");
		free(ALGORITMO_PLANIFICACION);
		free_claves_iniciales();
		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {
		free(ALGORITMO_PLANIFICACION);
		free_claves_iniciales();
		printf("Fallo el listen\n");
		exit(1);
	}
	printf("Servidor planificador escuchando!!!\n");

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

	// añadir el socket creado al conjunto maestro
	FD_SET(sockfd, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = sockfd; // por ahora es éste
	// bucle principal
	while (1) {

		read_fds = master; // copi el conjunto maestro como temporal
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) { //se encarga de llenar en read_fds todos los fd cliente que cambiaron
			perror("Error en select");
			free(ALGORITMO_PLANIFICACION);
			free_claves_iniciales();
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // recorro toda la bolsa read_fds
				if (i == sockfd) { // el que cambio es el fd del socket q cree y deje en listen, por ende, escucho algo ->
					// NUEVA CONEXION!!
					contador_id_esi++;
					int socketCliente = 0;
					if ((socketCliente = accept(sockfd,
							(struct sockaddr *) &their_addr, &sin_size))
							== -1) {
						perror("ERROR: Al ejecutar -> accept");
					} else {
						FD_SET(socketCliente, &master); // añadir al conjunto maestro ya q desde ahora en adelante lo vamos a usar para recibir
						if (socketCliente > fdmax) { // actualizar el máximo deacuerdo al fd del cliente
							fdmax = socketCliente;
						}
						printf("Se conecto el ESI de ID: %i\n",
								contador_id_esi);

						//Envio mensaje de saludo al ESI (usando su fd y su ID)
						int *idSocketCliente = (int *) malloc(sizeof(int32_t) * 2);
						idSocketCliente[0] = socketCliente;
						idSocketCliente[1] = contador_id_esi;


						atender_esi(idSocketCliente);

					}
				} else {
					//RECIBO DATOS DESDE ESI QUE PUEDEN SER RESPUESTA A UNA OPERACION O MENSAJE SALUDO
					int numbytes = 0;
					t_respuesta_para_planificador respuesta = {.id_tipo_respuesta = 0, .id_esi = 0,
									.mensaje = ""};

					if ((numbytes = recv(i, &respuesta, sizeof(respuesta), 0)) <= 0) {
						if (numbytes == 0) {
						// conexión cerrada
							printf("Se fue el ESI de fd: %d\n", i);
						} else {
							perror("ERROR: al recibir respuesta del ESI");
						}
						close(i); // si ya no conversare mas con el cliente, lo cierro
						FD_CLR(i, &master); // eliminar del conjunto maestro
						free_recurso(i); //liberamos los recursos que tenia ya que murio el esi
						remove_esi_by_fd(i); //TODO:Lo borramos de todos lados, no lo usaremos mas!(ver si lo tenemos que dejar al menos en terminado o no)
						if(aplico_algoritmo_ultimo()){
							continuar_comunicacion();
						}

					}else{
						if(respuesta.id_tipo_respuesta == 1){
							//Respuesta al primer saludo (todo nuevo)
//							printf("ESI id: %d envio saludo: %s\n",respuesta.id_esi,respuesta.mensaje);
							t_Esi* nuevo_esi = creo_esi(respuesta,i);
							agregar_en_Lista(LIST_READY,nuevo_esi);
							printf("ESI id: %d mando saludo: %s y se agrego a LISTA de READY\n",respuesta.id_esi,respuesta.mensaje);
							if(aplico_algoritmo_primer_ingreso()){
								continuar_comunicacion();
							}
						}
						if(respuesta.id_tipo_respuesta == 2){
							//Respuesta de una operacion que le pedi
							printf("ESI id: %d envio respuesta: %s\n",respuesta.id_esi,respuesta.mensaje);
							if(aplico_algoritmo()){
								continuar_comunicacion();
							}

						}

						//TODO:posiblemente si un ESI es ABORTADO pueda entrar por aca, analizar el caso
						if(respuesta.id_tipo_respuesta == 3){
							//Respuesta de que termino de leer las lineas
							printf("ESI id: %d envio respuesta: %s, nos despedimos de el!\n",respuesta.id_esi,respuesta.mensaje);
							close(i); // si ya no conversare mas con el cliente, lo cierro
							FD_CLR(i, &master); // eliminar del conjunto maestro
							cambio_de_lista(LIST_EXECUTE,LIST_FINISHED,respuesta.id_esi); //esta lo saca de ready y lo encola el terminado
							free_recurso(i);
							if(aplico_algoritmo_ultimo()){
								continuar_comunicacion();
							}

						}
					}

				}
			}
		}
	}
	free(ALGORITMO_PLANIFICACION);
	free_claves_iniciales();
	close(sockfd);
}

////paso de bloqueado a listo todos los esis que pedian esa clave
//void move_all_esi_bloqueado_listo(char* clave){
//
//	bool _esElid(t_nodoBloqueado* nodoBloqueado) { return (strcmp(nodoBloqueado->clave,clave)==0);}
//	int cant_esis_mover = 0;
//
//	if(list_find(LIST_BLOCKED, (void*)_esElid) != NULL){
//		cant_esis_mover = list_count_satisfying(LIST_BLOCKED, (void*)_esElid);
//	}
//	int contador = 0;
//	while (contador < cant_esis_mover){
//		t_nodoBloqueado* nodoBloqueado = list_find(LIST_BLOCKED,(void*) _esElid);
//		list_remove_by_condition(LIST_BLOCKED,(void*) _esElid);
//		t_Esi* esi = nodoBloqueado->esi;
//		list_add(LIST_READY,esi);
//		contador++;
//	}
//
//}
