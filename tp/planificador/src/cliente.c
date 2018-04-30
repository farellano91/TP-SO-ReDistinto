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
		free(IP_CONFIG_COORDINADOR);
		pthread_exit(NULL);
	}

	their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
	their_addr.sin_port = htons(PUERTO_CONFIG_COORDINADOR);  // short, Ordenación de bytes de la red
	their_addr.sin_addr.s_addr = inet_addr(IP_CONFIG_COORDINADOR);  //toma la ip directo

	memset(&(their_addr.sin_zero), 0, 8); // poner a cero el resto de la estructura

	if (connect(sockfd, (struct sockaddr *) &their_addr,
			sizeof(struct sockaddr)) == -1) {
		perror("No se pudo conectar al coordinador");
		free(IP_CONFIG_COORDINADOR);
		pthread_exit(NULL);
	}

	free(IP_CONFIG_COORDINADOR);
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

			}

			switch (id_operacion) {
			case 1:
				printf("Coordinador envio GET clave: %s del ESI ID: %d\n",clave,id_esi);
				//Controlo si get es sobre un recurso tomado (osea dentro de LIST_ESI_BLOQUEADOR para un unico ESI PAG.10)
				if(find_recurso_by_clave(clave)){

					//muevo de execute a block al ESI
					bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
					t_Esi* esi_buscado = list_find(LIST_EXECUTE,(void*) _esElid);
					esi_buscado->status=1;
					printf("Marco como bloqueado al ESI ID:%d\n",id_esi);

					//envio mensaje de ejecutado 1:falle , 2:ok , 3: ok pero te bloqueaste
					send_mensaje(fdCoordinador,3);
				}else{
					//registro la clave y continua (cargo en lis_esi_bloqueador)
					bool _esElid(t_Esi* un_esi) { return un_esi->id == id_esi;}
					t_Esi* esi_buscado = list_find(LIST_EXECUTE,(void*) _esElid);
					t_esiBloqueador* esi_bloqueador = get_esi_bloqueador(esi_buscado,clave);
					list_add(LIST_ESI_BLOQUEADOR,esi_bloqueador);

					printf("Registro que ahora la clave:%s lo tomo el ESI ID:%d\n",clave,id_esi);

					//envio mensaje de ejecutado 1:falle , 2:ok , 3: ok pero te bloqueaste
					send_mensaje(fdCoordinador,2);
				}
				break;
			case 3:

				printf("Coordinador envio STORE clave: %s del ESI ID: %d\n",clave,id_esi);
				//libero el recurso (borro de lis_esi_bloqueador el esi q corresponda)
				//TODO:por ahora se supone que solo puedo hacer STORE de los recursos que tome
				libero_recurso_by_clave_id(clave,id_esi);

				//paso de bloqueado a listo todos los ESIs que querian esa clave
				move_esi_from_bloqueado_to_listo(clave);

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


//busco si el recurso se encuentra tomado
bool find_recurso_by_clave(char* clave){

	bool resultado = false;
	bool _esElidClave(t_esiBloqueador* esi_bloqueador) { return (strcmp(esi_bloqueador->clave,clave)==0);}

	if(!list_is_empty(LIST_ESI_BLOQUEADOR) &&
			list_find(LIST_ESI_BLOQUEADOR, (void*)_esElidClave) != NULL){
		//Ya esta tomado ese recurso
		resultado = true;
	}
	return resultado;
}

void libero_recurso_by_clave_id(char* clave,int id_esi){
	bool _esElidClave(t_esiBloqueador* esi_bloqueador) { return (esi_bloqueador->esi->id == id_esi) && (strcmp(esi_bloqueador->clave,clave)==0);}

	if(!list_is_empty(LIST_ESI_BLOQUEADOR) &&
			list_find(LIST_ESI_BLOQUEADOR, (void*)_esElidClave) != NULL){

		//Solo lo saco de la lista
		list_remove_by_condition(LIST_ESI_BLOQUEADOR,(void*)_esElidClave);
		printf("Libero la clave:%s que tenia tomado el ESI ID:%d\n",clave,id_esi);
	}else{
		printf("No hay clave para liberar del ESI ID:%d\n",id_esi);
	}

}


