#include "cliente.h"

//PD:Si ejecuto esto la ip del coordinador ya lo libere ;)
void saludo_inicial_coordinador(int sockfd) {

	//Recibo saludo
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(sockfd, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del saludo\n");
		//MUERO
		exit(1);
	}
	char* mensajeSaludoRecibido = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(sockfd, mensajeSaludoRecibido, longitud, 0)) == -1) {
		printf("No se pudo recibir saludo\n");
		//MUERO
		exit(1);
	}

	printf("Saludo recibido: %s\n", mensajeSaludoRecibido);



	//Envio saludo
	char * mensajeSaludoEnviado = malloc(sizeof(char) * 100);
	strcpy(mensajeSaludoEnviado, "Hola, soy el PLANIFICADOREITOR");
	mensajeSaludoEnviado[strlen(mensajeSaludoEnviado)] = '\0';

	int32_t longitud_mensaje = strlen(mensajeSaludoEnviado) + 1;

	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),mensajeSaludoEnviado,longitud_mensaje);

	if (send(sockfd, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		perror("recv");
		printf("No se pudo enviar saludo\n");
		exit(1);
	}
	printf("Saludo enviado correctamente\n");

	free(bufferEnvio);
	free(mensajeSaludoEnviado);

	free(mensajeSaludoRecibido);

}

int conectar_coodinador() {

	//Creamos un socket
	int sockfd;
	struct sockaddr_in their_addr; // información de la dirección de destino

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		free(ip_config_coordinador);
		pthread_exit(NULL);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(puerto_config_coordinador);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(ip_config_coordinador);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("No se pudo conectar al coordinador");
		free(ip_config_coordinador);
		pthread_exit(NULL);
	}

	free(ip_config_coordinador);
	return (sockfd);
}

void recibirInfoCoordinador() {

	//SALUDOS DE CONEXION CON EL COORDINADOR
	int fdCoordinador = conectar_coodinador();
	saludo_inicial_coordinador(fdCoordinador);

	int numbytes = 0;

	while (1) {
		//recibo: ID_OPERACION,ID_ESI,LENG_CLAVE,CLAVE
		int32_t id_operacion = 0;
		int32_t id_esi = 0;
		int32_t leng_clave = 0;
		if ((numbytes = recv(fdCoordinador, &id_operacion,sizeof(int32_t), 0)) <= 0) {
			if(numbytes < 0 ){
				puts("Error al recibir tarea desde el coordinador");
			}else{
				puts("Se fue el coordinador");
			}
			break;
		} else {
			printf("Recibi operacion tipo: %d del coordinador\n",id_operacion);
			//recibo el id_esi y len_clave
			if ((numbytes = recv(fdCoordinador, &id_esi, sizeof(int32_t), 0)) <= 0) {
				if (numbytes < 0) {
					puts("Error al recibir el id del esi de la tarea que mando el coordinador");
				} else {
					puts("Se fue el coordinador");
				}

			} else {
				if ((numbytes = recv(fdCoordinador, &leng_clave, sizeof(int32_t), 0)) <= 0) {
					if (numbytes < 0) {
						puts("Error al recibir la longitud de la clave de la tarea que mando el coordinador");
					} else {
						puts("Se fue el coordinador");
					}
				} else {
					printf("Recibi id de esi:%d y longitud de clave:%d\n",id_esi,leng_clave);
				}
			}

			char* clave = malloc(sizeof(char)*leng_clave);

			//recibo ahora si la clave
			if ((numbytes = recv(fdCoordinador,clave,sizeof(char)*leng_clave, 0)) <= 0) {
				if (numbytes < 0) {
					puts("Error al recibir la clave de la tarea que mando el coordinador");
				} else {
					puts("Se fue el coordinador");
				}

			} else {
				printf("Recibi la clave %s correctamente\n",clave);
			}

			switch (id_operacion) {
			case 1:
				puts("Recibi un GET!!!!!!!!!!!!!");
				//Controlo si get es sobre un recurso tomado (osea dentro de list_esi_bloqueador para un unico ESI PAG.10)
				if(find_recurso_by_clave_id(clave,id_esi)){

					//muevo de execute a block al ESI
					bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
					t_Esi* esi_buscado = list_find(list_execute,(void*) _esElid);
					list_remove_by_condition(list_execute,(void*) _esElid);
					t_nodoBloqueado* esi_bloqueado = get_nodo_bloqueado(esi_buscado,clave);
					list_add(list_blocked,esi_bloqueado);

					printf("Muevo de EJECUCION a BLOQUEADO al ESI ID:%d\n",id_esi);

					//envio mensaje de que se bloqueo ese ESI
					send_mensaje(fdCoordinador,3);
				}else{
					//registro la clave y continua (cargo en lis_esi_bloqueador)
					bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
					t_Esi* esi_buscado = list_find(list_execute,(void*) _esElid);
					t_esiBloqueador* esi_bloqueador = get_esi_bloqueador(esi_buscado,clave);
					list_add(list_esi_bloqueador,esi_bloqueador);

					printf("Registro que ahora la clave:%s lo tomo el ESI ID:%d\n",clave,id_esi);

					//envio mensaje de ejecutado 1:falle , 2:ok , 3: ok pero te bloqueaste
					send_mensaje(fdCoordinador,2);
				}
				break;
			case 3:
				puts("Recibi un STORE!!!!!!!!!!!");
				//libero el recurso (borro de lis_esi_bloqueador el esi q corresponda)
				//TODO:por ahora se supone que solo puedo hacer STORE de los recursos que tome
				libero_recurso_by_clave_id(clave,id_esi);

				//paso de bloqueado a listo todos los ESIs que querian esa clave
				move_all_esi_bloqueado_listo(clave);

				//envio mensaje de ejecutado 1:falle , 2:ok , 3: ok pero te bloqueaste
				send_mensaje(fdCoordinador,2);
				break;

			}
			free(clave);
		}

	}
}


void send_mensaje(int fdCoordinador,int tipo_respuesta){
	if (send(fdCoordinador, &tipo_respuesta,sizeof(int32_t), 0) == -1) {
		perror("recv");
		printf("No se pudo enviar el resultado del GET o STORE\n");
		exit(1);
	}
	printf("Envié respuesta de la ejecucion de la operacion al coordinador\n");

}


bool find_recurso_by_clave_id(char* clave,int id_esi){

	bool resultado = false;
	bool _esElidClave(t_esiBloqueador* esi_bloqueador) { return (esi_bloqueador->esi->id == id_esi) && (strcmp(esi_bloqueador->clave,clave)==0);}

	if(!list_is_empty(list_esi_bloqueador) &&
			list_find(list_esi_bloqueador, (void*)_esElidClave) != NULL){
		//Ya esta tomado ese recurso
		resultado = true;
	}
	return resultado;
}

void libero_recurso_by_clave_id(char* clave,int id_esi){
	bool _esElidClave(t_esiBloqueador* esi_bloqueador) { return (esi_bloqueador->esi->id == id_esi) && (strcmp(esi_bloqueador->clave,clave)==0);}

	if(!list_is_empty(list_esi_bloqueador) &&
			list_find(list_esi_bloqueador, (void*)_esElidClave) != NULL){

		//Solo lo saco de la lista
		list_remove_by_condition(list_esi_bloqueador,(void*)_esElidClave);
		printf("Libero la clave:%s que tenia tomado el ESI ID:%d\n",clave,id_esi);
	}
}

void move_all_esi_bloqueado_listo(char* clave){

	bool _esElid(t_nodoBloqueado* nodoBloqueado) { return (strcmp(nodoBloqueado->clave,clave));}
	int cant_esis_mover = 0;

	if(list_find(list_blocked, (void*)_esElid) != NULL){
		cant_esis_mover = list_count_satisfying(list_blocked, (void*)_esElid);
	}
	int contador = 0;
	while (contador < cant_esis_mover){
		t_nodoBloqueado* nodoBloqueado = list_find(list_blocked,(void*) _esElid);
		list_remove_by_condition(list_blocked,(void*) _esElid);
		t_Esi* esi = nodoBloqueado->esi;
		list_add(list_ready,esi);
		contador++;
	}

}
