
#include "funcionalidad_instancia.h"

void intHandler(int dummy) {
	if (dummy != 0) {
		char* msg = string_from_format("Finalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		log_error(logger, msg);
		free(msg);
		free_estruct_admin();
		free_parametros_config();
		exit(dummy);

	}
}

void get_parametros_config(char* path){
//	t_config* config = config_create("config_prueba_algo_reem_comp_inst1.cfg");
	t_config* config = config_create(path);
	if (!config) {
		printf("No encuentro el archivo config\n");
		config_destroy(config);
		//MUERO
		exit(1);
	}

	PUERTO_CONFIG_COORDINADOR = config_get_int_value(config,"PUERTO_CONFIG_COORDINADOR");

	INTERVALO_DUMP = config_get_int_value(config,"INTERVALO_DUMP");

	IP_CONFIG_COORDINADOR = malloc(sizeof(char) * 100);
	strcpy(IP_CONFIG_COORDINADOR,config_get_string_value(config, "IP_CONFIG_COORDINADOR"));
	IP_CONFIG_COORDINADOR[strlen(IP_CONFIG_COORDINADOR)] = '\0';

	ALGORITMO_REEMPLAZO = malloc(sizeof(char) * 100);
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
	ALGORITMO_REEMPLAZO[strlen(ALGORITMO_REEMPLAZO)] = '\0';

	PUNTO_MONTAJE = malloc(sizeof(char) * 100);
	strcpy(PUNTO_MONTAJE,config_get_string_value(config, "PUNTO_MONTAJE"));
	PUNTO_MONTAJE[strlen(PUNTO_MONTAJE)] = '\0';

	NOMBRE_INSTANCIA = malloc(sizeof(char) * 100);
	strcpy(NOMBRE_INSTANCIA,config_get_string_value(config, "NOMBRE_INSTANCIA"));
	NOMBRE_INSTANCIA[strlen(NOMBRE_INSTANCIA)] = '\0';

	char* aux = string_from_format("log-%s.log", NOMBRE_INSTANCIA);

	logger = log_create(aux, NOMBRE_INSTANCIA, 1, LOG_LEVEL_INFO);

	free(aux);

	config_destroy(config);
}

void free_parametros_config(){

	free(IP_CONFIG_COORDINADOR);
	free(ALGORITMO_REEMPLAZO);
	free(PUNTO_MONTAJE);
	free(NOMBRE_INSTANCIA);
	log_destroy(logger);
}

void free_algo_punt_nom(){

	free(ALGORITMO_REEMPLAZO);
	free(PUNTO_MONTAJE);
	free(NOMBRE_INSTANCIA);
}

void free_registro_tabla_entrada(t_registro_tabla_entrada* registro){
	free(registro->clave);
	free(registro);
}

void free_registro_diccionario_entrada(t_registro_diccionario_entrada* registro){
	free(registro);
}

void free_estruct_admin(){

	if(!list_is_empty(TABLA_ENTRADA)){
		list_destroy_and_destroy_elements(TABLA_ENTRADA,(void*)free_registro_tabla_entrada);
	}
	if(!dictionary_is_empty(DICCIONARITY_ENTRADA)){
		dictionary_clean_and_destroy_elements(DICCIONARITY_ENTRADA,(void*)free_registro_diccionario_entrada);
	}

	int i = 0;
	for(i= 0;i<CANT_ENTRADA;i++){
		free(STORAGE[i]);
	}
	free(STORAGE);
}

//envio mi nuevo tamaño y resultado de la operacion
void envio_resultado_al_coordinador(int sockfd,int resultado){
	int32_t espacio_libre = 0;
	if(resultado == OK_SET_INSTANCIA){
		espacio_libre = obtener_espacio_libre();
	}
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&espacio_libre,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&resultado ,sizeof(int32_t) );

	if(send(sockfd,bufferEnvio, sizeof(int32_t)*2 , 0) == -1) {
		log_error(logger, "No se puede enviar el resultado al coordinador");
		free(bufferEnvio);
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	free(bufferEnvio);
	log_info(logger, "Envie mi resultado correctamente");
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
}


void recibo_mensaje_aceptacion(int fd_coordinador){
	int numbytes = 0;
	int32_t resultado_aceptacion = 0;
	if ((numbytes = recv(fd_coordinador, &resultado_aceptacion, sizeof(int32_t), 0)) <= 0) {
		if(numbytes == 0){
			log_error(logger, "Se desconecto el coordinador");
		}else{
			log_error(logger, "No se pudo recibir el resultado de la aceptacion");
		}
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	if(resultado_aceptacion == 1){
		//me rechazaron por nombre repetido
		log_error(logger, "Me rechazaron por nombre repetido");
		free_algo_punt_nom();
		exit(1);
	}
	log_info(logger, "Me aceptaron y encolaron en la lista de instancias");

}
//recibe la linea, la procesa ... y retorna un valor
int recibo_sentencia(int fd_coordinador){
	int32_t long_clave = 0;
	int32_t long_valor = 0;
	int32_t tipo_operacion = 0;
	int32_t numbytes = 0;
	int respuesta = 0;

	if ((numbytes = recv(fd_coordinador, &tipo_operacion, sizeof(int32_t), 0)) <= 0) {
			log_error(logger, "Coordinador desconectado");
			free_algo_punt_nom();
			free_estruct_admin();
			close(fd_coordinador);
			exit(1);
	}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	/*PROCESO.....*/
	if(tipo_operacion == SET){ //SET CLAVE VALOR

		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			log_error(logger, "No se pudo recibir le tamaño de la clave");
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			log_error(logger, "No se pudo recibir la clave");
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		if ((numbytes = recv(fd_coordinador, &long_valor, sizeof(int32_t), 0)) <= 0) {
			log_error(logger, "No se pudo recibir el tamaño del valor");
			free_algo_punt_nom();
			free(clave_recibida);
			close(fd_coordinador);
			exit(1);
		}

		char* valor_recibido = malloc(sizeof(char)*long_valor);
		if ((numbytes = recv(fd_coordinador, valor_recibido, long_valor, 0)) <= 0) {
			if(numbytes == 0){
				log_error(logger, "Se desconecto el coordinador");
			}else{
				log_error(logger, "No se pudo recibir el valor de la operacion");
			}
			free(valor_recibido);
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* aux = string_from_format("Recibi para hacer SET clave: %s valor: %s",clave_recibida,valor_recibido);
		log_info(logger, aux);
		free(aux);
		respuesta = ejecuto_set(clave_recibida,valor_recibido,fd_coordinador);
		free(valor_recibido);
		free(clave_recibida);

	}
	if(tipo_operacion == STORE){ //STORE CLAVE
		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			log_error(logger, "No se pudo recibir el tamaño de la clave");
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			log_error(logger, "No se pudo recibir la clave");
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}
		char* aux = string_from_format("Recibi para hacer STORE clave: %s",clave_recibida);
		log_info(logger, aux);
		free(aux);

		respuesta = ejecuto_store(clave_recibida);

		if(respuesta == OK_STORE_INSTANCIA){
			actualizo_cant_operaciones(clave_recibida);
		}

		free(clave_recibida);
	}
	if(tipo_operacion == COMPACTACION_LOCAL){
		compactar_ahora();
		pthread_mutex_unlock(&MUTEX_INSTANCIA);
		return COMPACTACION_LOCAL;
	}
	if(tipo_operacion == STATUS){
		 //STATUS CLAVE
		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			log_error(logger, "No se pudo recibir le tamaño de la clave");
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			log_error(logger, "No se pudo recibir la clave");
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}
		char* aux = string_from_format("Recibi para hacer STATUS clave: %s",clave_recibida);
		log_info(logger, aux);
        free(aux);

		//envio respuesta
		int32_t espacio_libre = 0;
		int32_t resultado = OK_STATUS;
		char* valor = get_valor_by_clave(clave_recibida);

		if(valor == NULL){
			//significa q la clave q me piden ya no la tengo mas, la sobreescribi
			valor = malloc(sizeof(char)* TAMANIO_ENTRADA* CANT_ENTRADA);
			strcpy(valor,"No tenemos el valor");
			valor[strlen(valor)] = '\0';
		}
		int32_t leng_valor = strlen(valor) + 1 ;

		void* bufferEnvio = malloc(sizeof(int32_t)*3 +leng_valor );
		memcpy(bufferEnvio,&espacio_libre,sizeof(int32_t));
		memcpy(bufferEnvio + sizeof(int32_t),&resultado ,sizeof(int32_t) );
		memcpy(bufferEnvio + sizeof(int32_t)*2,&leng_valor ,sizeof(int32_t) );
		memcpy(bufferEnvio + sizeof(int32_t)*3,valor,leng_valor);

		if(send(fd_coordinador,bufferEnvio,sizeof(int32_t)*3 +leng_valor , 0) == -1) {
			log_error(logger, "No se puede enviar el resultado del status al coordinador");
			free_algo_punt_nom();
			free_estruct_admin();
			exit(1);
		}else{
			char* aux = string_from_format("El valor de la clave %s es %s",clave_recibida,valor);
			log_info(logger, aux);
			free(aux);
		}

		free(clave_recibida);
		free(bufferEnvio);
		free(valor);
		pthread_mutex_unlock(&MUTEX_INSTANCIA);
		return OK_STATUS;
	}

	return respuesta;
}

int ejecuto_set(char* clave_recibida,char* valor_recibido,int fd_coordinador){

	bool control_final = false; //esto me sirve para saber si al final pude o no insertar lo pedido

	int entradas_necesarias = get_cant_entradas_by_valor(valor_recibido);
	char* aux = string_from_format("El valor de la clave recibida ocuparia %d entradas",entradas_necesarias);
	log_info(logger, aux);
	free(aux);

	//si la clave es una clave existente libero las entradas viejas
	if(clave_existente(clave_recibida)){

		//controlo que no aya crecido
		if(get_cant_entradas_by_clave(clave_recibida) < entradas_necesarias){
			log_error(logger, "La clave existe, pero el nuevo valor ocupa más entradas que la original");
			return FALLO_ENTRADA_MAS_GRANDE;
		}

		log_info(logger, "La clave existe, entonces paso a limpiar sus entradas para poder sobreescribirla");
		libero_entradas_by_clave(clave_recibida);
	}

	//si la clave es nueva
	int entrada_inicial = -1;
	int cant_espacio_disponibles = obtener_espacio_libre();
	if(cant_espacio_disponibles >= entradas_necesarias){
		if(son_contiguos(entradas_necesarias,&entrada_inicial)){
			control_final =guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}else{
			compacto(&entrada_inicial,fd_coordinador);
			control_final =guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}
	}else{
		if(!aplico_reemplazo(entradas_necesarias - cant_espacio_disponibles)){//reemplaza tanta cantidad de claves como espacios necesite
			return FALLO_CASO_BORDE;//caso donde no hay ninguna entrada para liberar
		}
		if(son_contiguos(entradas_necesarias,&entrada_inicial)){
			control_final =guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}else{
			compacto(&entrada_inicial,fd_coordinador);
			control_final = guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}
	}

	if(control_final){
		actualizo_cant_operaciones(clave_recibida);
		return OK_SET_INSTANCIA;
	}else{
		//no pude insertar a pesar de aplicar todas las tecnicas
		return FALLO_CASO_BORDE;
	}


}

//busca la cant de entradas donde se encuentra la clave
int get_cant_entradas_by_clave(char* clave){
	bool _esLaClave(t_registro_tabla_entrada* reg) { return strcmp(reg->clave,clave)==0;}
	int32_t cant_entradas = 0;

	if(!list_is_empty(TABLA_ENTRADA)){
		cant_entradas = list_count_satisfying(TABLA_ENTRADA, (void*)_esLaClave);
	}
	return cant_entradas;
}

int get_cant_entradas_by_valor(char* valor){
	int entradas_necesarias = 0;
	if(( strlen(valor) + 1 ) <= TAMANIO_ENTRADA){
		entradas_necesarias = 1;
	}else{
		entradas_necesarias =  (strlen(valor) / (TAMANIO_ENTRADA - 1 ));
		if((strlen(valor) % (TAMANIO_ENTRADA - 1 ) ) != 0){
			entradas_necesarias++;
		}
	}
	return entradas_necesarias;
}

void libero_entradas_by_clave(char * clave_recibida){
	void _porClaveLiberoEntrada(t_registro_tabla_entrada* registro) {
		if(strcmp(registro->clave, clave_recibida) == 0){
			libero_entrada(registro->numero_entrada);
		}
	}
	list_iterate(TABLA_ENTRADA,(void*) _porClaveLiberoEntrada);

}

//deja libre la entrada de numero recibido y actualiza las demas estructuras
void libero_entrada(int numeroEntrada){
	//limpio storage
	strcpy(STORAGE[numeroEntrada],"");
	//limpio tabla de entradas
	bool _esEntrada(t_registro_tabla_entrada* entrada) { return (entrada->numero_entrada == numeroEntrada);}
	list_remove_and_destroy_by_condition(TABLA_ENTRADA,(void*)_esEntrada,(void*)free_registro_tabla_entrada);

	//Limpio diccionario de entradas (OJO no borro solo limpio las entradas)
	char* key = string_itoa(numeroEntrada);
	t_registro_diccionario_entrada * diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
	diccionario->cant_operaciones = 0;
	diccionario->libre = 1;
	diccionario->tamanio_libre = TAMANIO_ENTRADA;

	free(key);
}


void delete_file_dump(int numeroEntrada){
	bool _esEntrada(t_registro_tabla_entrada* entrada) { return (entrada->numero_entrada == numeroEntrada);}
	t_registro_tabla_entrada* registro = list_find(TABLA_ENTRADA,(void*)_esEntrada);
	int ret;
	char * path = malloc(strlen(PUNTO_MONTAJE) +strlen(registro->clave) + strlen(".txt") + 1 );//+1 for the null-terminator
	strcpy(path,PUNTO_MONTAJE);
	strcat(path, registro->clave);
	strcat(path, ".txt");
	ret = remove(path);

	if(ret == 0) {
	  char* aux = string_from_format("Borramos el archivo del path:%s ",path);
	  log_info(logger, aux);
	  free(aux);
	}
	free(path);
}

//busca si existe la clave en mi tabla de entradas
bool clave_existente(char * clave_recibida) {
	if (!list_is_empty(TABLA_ENTRADA)) {
		bool _porClave(t_registro_tabla_entrada* registro) {
			return (strcmp(registro->clave, clave_recibida) == 0);
		}
		t_registro_tabla_entrada* registro_buscado = list_find(TABLA_ENTRADA,
				(void*) _porClave);
		if (registro_buscado != NULL) {
			return true;
		}
	}

	return false;
}

void compacto(int* entrada_inicial,int fd_coordinador) {

	//aviso al coordinador q compacten todas las instancias
	notifico_inicio_compactacion(fd_coordinador);

	compactar_ahora();

	//informa el primer vacio que tenemos
	int i;
	for (i = 0; i < CANT_ENTRADA; i++) {
		char* key = string_itoa(i);
		t_registro_diccionario_entrada* diccionario = dictionary_get(
				DICCIONARITY_ENTRADA, key);
		if (diccionario->libre == 1) {
			*entrada_inicial = i;
			break;
		}
	}
}

void notifico_inicio_compactacion(int fd_coordinador){
	int32_t compacto = COMPACTACION_GLOBAL;
	int32_t espacio_libre = 0;//envio 0 ya q el coordinador va a ignorar esto para este tipo de mensaje
	void* bufferEnvio = malloc(sizeof(int32_t)*2);
	memcpy(bufferEnvio,&espacio_libre,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),&compacto ,sizeof(int32_t) );

	if(send(fd_coordinador,bufferEnvio, sizeof(int32_t)*2 , 0) == -1) {
		log_error(logger, "No se puede enviar el pedido de compactacion global al coordinador");
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	log_info(logger, "Envio de solicitud de compactacion global al coordinador exitoso");
}

void compactar_ahora(){
	log_info(logger, "Inicio compactacion...");
	int i;
	for(i = 0 ; i <CANT_ENTRADA;i++){
		char* key = string_itoa(i);
		t_registro_diccionario_entrada* diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		if(diccionario->libre == 0){
			int entrada_superior_vacia = get_entrada_superior_vacia(i);
			if(entrada_superior_vacia >=  0){
				char* aux = string_from_format("Se mueve la entrada n°:%d al n°:%d",i,entrada_superior_vacia);
				log_info(logger, aux);
				free(aux);
				//me muevo a esa entrada
				cambio_entrada(i,entrada_superior_vacia);
			}
		}
	}

	log_info(logger, "Fin compactacion...");
	print_storage();
}

//busca la entrada superior que este vacia
int get_entrada_superior_vacia(int entrada){
	int resultado = -1;
	int i;
	for(i = (entrada-1); i >= 0 ; i--){
		char* key = string_itoa(i);
		t_registro_diccionario_entrada* diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		if(diccionario->libre == 1){
			resultado = i;
		}else{
			break;
		}
	}
	return resultado;
}

void cambio_entrada(int entrada_desde,int entrada_hasta){
	bool _esEntrada(t_registro_tabla_entrada* registro_tabla) {
		return (registro_tabla->numero_entrada == entrada_desde);
	}
	t_registro_tabla_entrada* tabla = list_find(TABLA_ENTRADA,(void*)_esEntrada);

	//copio el alor desde al valor hasta
	strcpy(STORAGE[entrada_hasta],STORAGE[entrada_desde]);

	cargo_actualizo_tabla(tabla->clave,entrada_hasta,tabla->tamanio_valor);
	cargo_actualizo_diccionario(entrada_hasta,tabla->tamanio_valor);

	libero_entrada(entrada_desde);
}

//reemplaza tantas veces como entradas_necesarias - cant_espacio_disponibles
bool aplico_reemplazo(int cant_espacios_buscados){
	int i;
	int numeroEntrada = -1;
	log_info(logger, "Tengo que reemplazar por falta de espacio");
	if(cant_espacios_buscados > 0){
		for(i = 0 ; i < cant_espacios_buscados; i++){
			numeroEntrada = aplicarAlgoritmoReemplazo();
			//controlar q el numeroEntrada sea != -1
			if(numeroEntrada == -1){
				log_info(logger, "No hay ninguna entrada atomica para reemplazar");
				return false;
			}else{
				//borra el .txt que estaba
				delete_file_dump(numeroEntrada);
				libero_entrada(numeroEntrada);
				char* aux = string_from_format("Libero la entrada atomica NUMERO: %d",numeroEntrada);
				log_info(logger, aux);
				free(aux);
			}
		}
	}
	return true;
}


bool guardo_valor(int entrada_inicial,char* clave_recibida,char* valor_recibido,int entradas_necesarias){
	int i;
	int tamanio_contenido = 0;
	if(entradas_necesarias > obtener_espacio_libre()){

		//ordeno tabla por numero de entrada(nos sirve para cuando buscamos claves diferentes)
		order_tabla_by(TABLA_ENTRADA,(void*) by_numero_entrada);

		//es imposible insertar, caso borde
		return false;
	}else{
		if(entradas_necesarias > 1){
			for(i=0;i<entradas_necesarias;i++){

				tamanio_contenido = strlen(valor_recibido) + 1;

				if(tamanio_contenido > TAMANIO_ENTRADA){
					memcpy(STORAGE[entrada_inicial + i],valor_recibido,TAMANIO_ENTRADA-1);
					STORAGE[entrada_inicial + i][TAMANIO_ENTRADA-1]='\0';
					char* clave_auxiliar = string_substring_from(valor_recibido,TAMANIO_ENTRADA -1);
					strcpy(valor_recibido,clave_auxiliar);
					free(clave_auxiliar);

					//actualizo tabla
					cargo_actualizo_tabla(clave_recibida,entrada_inicial+i,TAMANIO_ENTRADA);
					//actualizo diccionario
					cargo_actualizo_diccionario(entrada_inicial+i,TAMANIO_ENTRADA);
				}else{
					memcpy(STORAGE[entrada_inicial + i],valor_recibido,tamanio_contenido);
					STORAGE[entrada_inicial + i][tamanio_contenido-1]='\0';
					//actualizo tabla
					cargo_actualizo_tabla(clave_recibida,entrada_inicial+i,tamanio_contenido);
					//actualizo diccionario
					cargo_actualizo_diccionario(entrada_inicial+i,tamanio_contenido);
				}

			}
		}else{
			strcpy(STORAGE[entrada_inicial],valor_recibido);
			//actualizo tabla
			cargo_actualizo_tabla(clave_recibida,entrada_inicial,strlen(valor_recibido) + 1);
			//actualizo diccionario
			cargo_actualizo_diccionario(entrada_inicial, strlen(valor_recibido) + 1);
		}
	}

	//ordeno tabla por numero de entrada(nos sirve para cuando buscamos claves diferentes)
	order_tabla_by(TABLA_ENTRADA,(void*) by_numero_entrada);


	print_storage();
	return true;
}

void print_storage(){
	int i;
	//imprimo el storage
	for(i = 0; i < CANT_ENTRADA;i++){
		//printf("[STORAGE[%d] = %s]\n",i,STORAGE[i]);
		char* aux = string_from_format("[STORAGE[%d] = %s]",i,STORAGE[i]);
		log_info(logger, aux);
		free(aux);
	}
}
bool by_numero_entrada(t_registro_tabla_entrada * registro_menor, t_registro_tabla_entrada * registro) {
	return ((registro_menor->numero_entrada) <= (registro->numero_entrada)); //agrego el = pork pasaba q para el mismo valor lo cambiaba de lugar cosa q estaba de mas
}

void order_tabla_by(t_list* tabla, void * funcion){
	list_sort(tabla, (void*) funcion);
}

bool son_contiguos(int entradas_necesarias, int* entrada_inicial){
	int contiguo = 0;
	int i;
	for(i = 0; i< dictionary_size(DICCIONARITY_ENTRADA);i++){
		char * key = string_itoa(i);
		t_registro_diccionario_entrada* registro = dictionary_get(DICCIONARITY_ENTRADA,key);
		if(registro->libre){
			contiguo++;
			if(contiguo==entradas_necesarias){
				*entrada_inicial = i + 1 - entradas_necesarias;
				free(key);
				return true;
			}
		}else{
			free(key);
			contiguo = 0;
		}
	}
	return false;

}

int ejecuto_store(char* clave_recibida){

	//si la clave-valor ya no existe en la instancia dado que se pizo: FALLO_INSTANCIA_CLAVE_SOBREESCRITA
	char* valor_del_storage = get_valor_by_clave(clave_recibida);
	if(valor_del_storage == NULL){
		log_error(logger, "No existe la clave-valor dado que alguna de sus entradas se reemplazo");
		return FALLO_INSTANCIA_CLAVE_SOBREESCRITA;
	}

	char* aux = string_from_format("El valor a guardar es:%s",valor_del_storage);
	log_info(logger, aux);
	free(aux);

	//guardo o actualizo el .txt
	char* path_archivo = malloc(strlen(clave_recibida) + 4 + strlen(PUNTO_MONTAJE) + 1);
	strcpy(path_archivo,PUNTO_MONTAJE);
	strcat(path_archivo,clave_recibida);
	strcat(path_archivo,".txt");
	path_archivo[strlen(path_archivo)] = '\0';

	create_or_update_file(path_archivo,valor_del_storage);

	//por ahora
	free(path_archivo);
	free(valor_del_storage);
	return OK_STORE_INSTANCIA;
}


void create_or_update_file(char *path_archivo, char * valor_del_storage){

	//open con crear o actualizar
	int fd = open(path_archivo, O_RDWR | O_CREAT | O_TRUNC, ( mode_t ) 0600);

	if (write(fd, valor_del_storage,strlen(valor_del_storage)) == -1)
	{
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}
	//cerrar
	close(fd);
}

//retorna todo el valor completo a partir de la clave
char* get_valor_by_clave(char * clave_recibida){

	char* valor_buscado = malloc(sizeof(char)* TAMANIO_ENTRADA* CANT_ENTRADA);
	strcpy(valor_buscado,"");

	bool _esClave(t_registro_tabla_entrada* entrada) { return strcmp(entrada->clave,clave_recibida)== 0;}
	t_list* tabla_entrada =  list_filter(TABLA_ENTRADA,(void*) _esClave);

	if(list_size(tabla_entrada) > 0){
		void _armoValor(t_registro_tabla_entrada* entrada) {
			strcat(valor_buscado,STORAGE[entrada->numero_entrada]);
		}
		list_iterate(tabla_entrada,(void*)_armoValor);
		valor_buscado[strlen(valor_buscado)] = '\0';
		list_destroy(tabla_entrada);
		return valor_buscado;
	}
	free(valor_buscado);
	list_destroy(tabla_entrada);
	return NULL;

}

void recibo_datos_entrada(int fd_coordinador){
	int numbytes = 0;
	if ((numbytes = recv(fd_coordinador, &TAMANIO_ENTRADA, sizeof(int32_t), 0)) == -1) {
		log_error(logger, "No se pudo recibir el tamaño de la entrada");
		free_algo_punt_nom();
		exit(1);
	}
	if ((numbytes = recv(fd_coordinador, &CANT_ENTRADA, sizeof(int32_t), 0)) == -1) {
		log_error(logger, "No se pudo recibir la cantidad de entradas");
		free_algo_punt_nom();
		exit(1);
	}
	char* aux = string_from_format("Recibi tamaño de entrada: %d y cantidad de entradas: %d correctamente",TAMANIO_ENTRADA,CANT_ENTRADA);
	log_info(logger, aux);
	free(aux);

}

//esto para q el coordinador puedo crear su t_instancia
void envio_datos(int fd_coordinador){

	int32_t espacio_libre = obtener_espacio_libre();

	int32_t longitud_mensaje = strlen(NOMBRE_INSTANCIA) + 1;
	void* bufferEnvio = malloc(sizeof(int32_t)*2 + sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),NOMBRE_INSTANCIA,longitud_mensaje);
	memcpy(bufferEnvio + sizeof(int32_t) + longitud_mensaje,&espacio_libre,sizeof(int32_t));


	if (send(fd_coordinador, bufferEnvio,sizeof(int32_t)*2 + sizeof(char)*longitud_mensaje, 0) == -1) {
		log_error(logger, "No pude enviar mis datos al coordinador");
		free(bufferEnvio);
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	char* aux = string_from_format("Envie mi nombre y cant. de entradas libres:%d correctamente",espacio_libre);
	log_info(logger, aux);
	free(aux);
	free(bufferEnvio);
}

//obtiene la cant. de entradas libres actual
int obtener_espacio_libre(){
	int espacio_total_disponible = 0;

	void _buscaEspacioLibre(char* _, t_registro_diccionario_entrada* registro) {
		if(registro->libre){
			espacio_total_disponible = espacio_total_disponible + 1;
		}

	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_buscaEspacioLibre);
	return espacio_total_disponible;
}

//obtiene el tamaño en byte libres actual
int obtener_tamanio_libre(){
	int tamanio_total_disponible = 0;

	void _buscaTamanioLibre(char* _, t_registro_diccionario_entrada* registro) {
		tamanio_total_disponible = tamanio_total_disponible + registro->tamanio_libre;
	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_buscaTamanioLibre);
	return tamanio_total_disponible;
}

//con los datos de entrada que recibi, puedo inicializar mis estructuras
void inicializo_estructuras(){

	PUNTERO_DIRECCION_CIRCULAR = 0;

	//inicializo diccionario de entrada
	DICCIONARITY_ENTRADA = create_diccionarity();

	//inicializo storage
	STORAGE = malloc(sizeof(char*)* CANT_ENTRADA);
	int i = 0;
	for(i= 0;i<CANT_ENTRADA;i++){
		STORAGE[i] = malloc(sizeof(char) * TAMANIO_ENTRADA); //es 100, osea 99 caractes posta
		strcpy(STORAGE[i],"");
        char* key = string_itoa(i);
		t_registro_diccionario_entrada* registro = get_new_registro_dic_entrada(1,0,TAMANIO_ENTRADA);
		dictionary_put(DICCIONARITY_ENTRADA,key,registro);
		free(key);
	}

	//inicializo tabla entradas
	TABLA_ENTRADA = create_list();


}

size_t getFilesize(const char* filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

//leer los archivos .txt creador a partir del dump para asi poder cargar mis estructuras administrativas
void reestablecer_datos(){
	log_info(logger, "Leo mis archivos previamente guardados si es que tengo");
	FILE *fp;
	char* nombre_archivo = malloc(sizeof(char)* 100);
	char* nombre_archivo_sin_salto = malloc(sizeof(char)* 100);

	int contador_archivos = 0;

	char *ls_with_path = malloc(strlen("/bin/ls ") + strlen(PUNTO_MONTAJE) + 1 );//+1 for the null-terminator
	strcpy(ls_with_path,"/bin/ls ");
	strcat(ls_with_path, PUNTO_MONTAJE);

	/* Abro la carpeta. */
	fp = popen(ls_with_path, "r");
	if (fp == NULL) {
		log_error(logger, "Error al tratar de abrir la carpeta" );
		free_algo_punt_nom();
		free_estruct_admin();
		free(ls_with_path);
		exit(1);
	}
	free(ls_with_path);
	/* Vemos que hay dentro de la carpeta */
	while (fgets(nombre_archivo,100, fp) != NULL) {
		//Nota: fgets le agrega un \n al nombre de la linea q lee
		memcpy(nombre_archivo_sin_salto,nombre_archivo,strlen(nombre_archivo)-1);
		nombre_archivo_sin_salto[strlen(nombre_archivo)-1]='\0';
		char* aux = string_from_format("Tenemos para reestablecer el archivo :%s", nombre_archivo_sin_salto);
		log_info(logger, aux);
		free(aux);
		contador_archivos ++;
		reestablesco_archivo(nombre_archivo_sin_salto);
	}
	if(contador_archivos == 0){
		log_info(logger, "No hay nada para reestablecer");
	}
	free(nombre_archivo_sin_salto);
	free(nombre_archivo);
	pclose(fp);
}

void reestablesco_archivo(char* nombre_archivo){
	char* path_archivo = malloc(strlen(nombre_archivo) + strlen(PUNTO_MONTAJE) + 1);
	strcpy(path_archivo,PUNTO_MONTAJE);
	strcat(path_archivo,nombre_archivo);
	path_archivo[strlen(path_archivo)] = '\0';

	size_t tamanio_contenido = getFilesize(path_archivo); //tamaño completo, osea "gato" => 4

	int fd = open(path_archivo, O_RDONLY, 0);
	void* mmappedData = mmap(NULL, tamanio_contenido, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
	if ( mmappedData == MAP_FAILED ){
		log_error(logger, "Error al tratar de usar mmap");
		free(path_archivo);
		free_algo_punt_nom();
		free_estruct_admin();
		close(fd);
		exit(1);
	}
	int32_t len_clave = strlen(nombre_archivo) - 4 + 1;
	char* clave = malloc(len_clave);
	memcpy(clave,nombre_archivo,strlen(nombre_archivo) - 4); //copio sin el .txt
	clave[len_clave - 1] = '\0';


	int32_t len_valor = tamanio_contenido + 1;
	char* valor = malloc(len_valor);
	memcpy(valor,mmappedData,tamanio_contenido);
	valor[tamanio_contenido] = '\0';


	if (munmap(mmappedData, tamanio_contenido) == -1){
		log_error(logger, "Error al tratar de liberar memoria de un mmap");
		close(fd);
		free(path_archivo);
		free_algo_punt_nom();
		free_estruct_admin();
		free(clave);
		free(valor);
		exit(1);
	}
	//cargo mis estructuras
	cargar_estructuras(clave,valor,len_valor);

	free(path_archivo);
	free(clave);
	free(valor);


	close(fd);
}

void cargar_estructuras(char* clave,char* valor,int tamanio_contenido){
	int i = 0;
	int total_entradas_necesarias = 0;
	if(( strlen(valor) + 1 ) <= TAMANIO_ENTRADA){
		total_entradas_necesarias = 1;
	}else{
		total_entradas_necesarias =  1 + (( strlen(valor) + 1 ) / TAMANIO_ENTRADA);
	}
	if(total_entradas_necesarias  <= CANT_ENTRADA){
		for(i = 0; i < CANT_ENTRADA; i++){
			if(strcmp(STORAGE[i],"")== 0){
				//esta vacio
				if(tamanio_contenido <= TAMANIO_ENTRADA){
					//entra en uno solo
					strcpy(STORAGE[i],valor);
					char* aux = string_from_format("Cargo en la entrada numero: %d el valor: %s",i,valor);
					log_info(logger, aux);
					free(aux);

					//actualizo tabla
					cargo_actualizo_tabla(clave,i,tamanio_contenido);
					//actualizo diccionario
					cargo_actualizo_diccionario(i,tamanio_contenido);

					break;
				}else{
					//no entra en uno solo, entoces los divido
					memcpy(STORAGE[i],valor,TAMANIO_ENTRADA-1);
					STORAGE[i][TAMANIO_ENTRADA-1]='\0';
					char* aux = string_from_format("Cargo en la entrada numero: %d el valor: %s",i,STORAGE[i]);
					log_info(logger, aux);
					free(aux);

					memcpy(valor,valor + TAMANIO_ENTRADA -1 ,tamanio_contenido);
					//tamanio_contenido = tamanio_contenido - TAMANIO_ENTRADA + 1;
					tamanio_contenido = strlen(valor) + 1;

					//actualizo tabla
					cargo_actualizo_tabla(clave,i,TAMANIO_ENTRADA);
					//actualizo diccionario
					cargo_actualizo_diccionario(i,TAMANIO_ENTRADA);

				}
			}
		}
	}else{
		char* aux = string_from_format("El archivo .txt: %s necesita más entradas de las que tengo en total, imposible reestablecerlo",clave);
		log_error(logger, aux);
		free(aux);
	}


}

//"lista"
void cargo_actualizo_tabla(char* clave,int numero_entrada,int tamanio_contenido){
	//busco si ya esta guardado ese numero de entrada
	bool _porNumeroEntrada(t_registro_tabla_entrada* registro) { return (registro->numero_entrada == numero_entrada);}
	t_registro_tabla_entrada* registro_buscado = list_find(TABLA_ENTRADA,(void*)_porNumeroEntrada);
	if(registro_buscado != NULL){
		//si ya esta guardado, entonces lo actualizo
		strcpy(registro_buscado->clave,clave);
		registro_buscado->tamanio_valor = tamanio_contenido;
		char* aux = string_from_format("Actualizo en mi tabla la entrada:%d clave:%s tamaño del valor:%d",numero_entrada,clave,tamanio_contenido);
		log_info(logger, aux);
		free(aux);
	}else{
		//si no esta guardado, lo guardo por primera vez
		t_registro_tabla_entrada * nuevo_registro = get_new_registro_tabla_entrada(numero_entrada,clave,tamanio_contenido);
		list_add(TABLA_ENTRADA,nuevo_registro);
		char* aux = string_from_format("Cargo en mi tabla la entrada:%d clave:%s tamaño del valor:%d",numero_entrada,clave,tamanio_contenido);
		log_info(logger, aux);
		free(aux);
	}
}

void cargo_actualizo_diccionario(int numero_entrada,int tamanio_contenido){
	char * key = string_itoa(numero_entrada);
	//buscamos si ya esta esa entrada guardada
	if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
		//existe la key en el diccionario
		t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		registro_diccionario->libre = 0;
		registro_diccionario->tamanio_libre = TAMANIO_ENTRADA - tamanio_contenido;
		char* aux = string_from_format("Actualizo en mi diccionario la entrada:%d-ocupada-cant operaciones:%d-tamaño libre de la entrada:%d",numero_entrada,registro_diccionario->cant_operaciones,registro_diccionario->tamanio_libre);
		log_info(logger, aux);
		free(aux);
	    free(key);
	}else{
		//no existe, lo crea
		t_registro_diccionario_entrada * registro_diccionario = get_new_registro_dic_entrada(0,0,(TAMANIO_ENTRADA - tamanio_contenido));
		dictionary_put(DICCIONARITY_ENTRADA,key,registro_diccionario);
		char* aux = string_from_format("Cargo en mi diccionario la entrada:%d-ocupada-cant operaciones:1-tamaño libre de la entrada:%d",numero_entrada,registro_diccionario->tamanio_libre);
		log_info(logger, aux);
		free(aux);
	}

}

//a la entrada con esta clave la seteo en 0 (porque se uso recien) y a las ademas entradas OCUPADAS las operaciones las sumo en +1
void actualizo_cant_operaciones(char* clave){
	//busco todas las entradas que corresponden a esta clave de mi TABLA DE ENTRADAS
	void _esClave(t_registro_tabla_entrada* registro) {
		char * key = string_itoa(registro->numero_entrada);
		if(strcmp(registro->clave, clave) == 0){
			//buscamos si ya esta esa entrada
			if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
				//existe la key en el diccionario
				t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
				registro_diccionario->cant_operaciones = 0; //porque fue usada recien
			}

		}else{
			//aunmento +1 en operaciones
			if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
				//existe la key en el diccionario
				t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
				registro_diccionario->cant_operaciones++; //porque no la use
			}
		}
		free(key);
	}
	list_iterate(TABLA_ENTRADA,(void*)_esClave);


	//SOLO PARA CONTROLAR Q TODO FUNCIONE :Imprimo como quedo luego de la operacion de store/set---------------------
	//print_diccionario();

}

void print_diccionario(){
	void _muestroEstadoOperaciones(char* key, t_registro_diccionario_entrada* diccionario) {
		//printf("[La entrada numero: %s esta libre: %d y lleva: %d operaciones]\n",key,diccionario->libre,diccionario->cant_operaciones);
		char* aux = string_from_format("[La entrada numero: %s esta libre: %d y lleva: %d operaciones]",key,diccionario->libre,diccionario->cant_operaciones);
		log_info(logger, aux);
		free(aux);
	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_muestroEstadoOperaciones);
}

t_dictionary* create_diccionarity(){
	t_dictionary * diccionarity = dictionary_create();
	return diccionarity;
}

t_list* create_list(){
	t_list * list = list_create();
	return list;
}

t_registro_diccionario_entrada* get_new_registro_dic_entrada(int libre,int cant_operaciones,int tamanio_libre){
	t_registro_diccionario_entrada* registro_dic_entrada = malloc(sizeof(t_registro_diccionario_entrada));
	registro_dic_entrada->cant_operaciones = cant_operaciones;
	registro_dic_entrada->libre = libre;
	registro_dic_entrada->tamanio_libre = tamanio_libre;

	return registro_dic_entrada;
}

t_registro_tabla_entrada* get_new_registro_tabla_entrada(int numero_entrada,char* clave,int tamanio_valor){
	t_registro_tabla_entrada* registro_tabla_entrada = malloc(sizeof(t_registro_tabla_entrada));
	registro_tabla_entrada->numero_entrada = numero_entrada;
	registro_tabla_entrada->tamanio_valor = tamanio_valor;
	int32_t len_clave = strlen(clave) + 1;
	registro_tabla_entrada->clave = malloc(sizeof(char) * len_clave);
	strcpy(registro_tabla_entrada->clave,clave);
	return registro_tabla_entrada;
}

void realizar_dump(){
	while(1){
		sleep(INTERVALO_DUMP);
		pthread_mutex_lock(&MUTEX_INSTANCIA);
		log_info(logger, "Comienzo dump....");
		//controlo si hay algo en la tabla de entrada
		if(!list_is_empty(TABLA_ENTRADA)){
			t_list* tabla_solo_claves = get_only_clave();
			void _aplicaSTORE(t_registro_tabla_entrada* una_entrada) {
				int resultado = ejecuto_store(una_entrada->clave);
				if(resultado == FALLO_INSTANCIA_CLAVE_SOBREESCRITA){
					char* aux = string_from_format("Fallo al hacer DUMP de la clave: %s",una_entrada->clave);
					log_error(logger, aux);
					free(aux);
				}
				char* aux = string_from_format("Dump de la clave: %s correctamente hecho",una_entrada->clave);
				log_info(logger, aux);
				free(aux);

			}
			list_iterate(tabla_solo_claves,(void*)_aplicaSTORE);
			list_destroy(tabla_solo_claves);
		}else{
			log_info(logger, "No hay entradas ocupadas para hacer DUMP");
		}
		log_info(logger, "Finalizo dump....");
		pthread_mutex_unlock(&MUTEX_INSTANCIA);
	}
}

//retorna una tabla de entradas solo con claves diferentes
t_list* get_only_clave(){
	t_registro_tabla_entrada * primero = list_get(TABLA_ENTRADA,0);
	int contador = 0;

	bool _claveDiferente(t_registro_tabla_entrada * una_Entrada){
		if((contador > 0)){
			if(strcmp(una_Entrada->clave,primero->clave) != 0){
				primero = una_Entrada;
				return true;
			}
			return false;
		}else{
			contador++;
			return true;
		}
	}

	return list_filter(TABLA_ENTRADA, (void*)_claveDiferente);
}


int aplicarAlgoritmoReemplazo(){

	t_list* listaEntradasAtomicas = filtrar_atomico();
	int respuesta = -1 ;
	if(list_is_empty(listaEntradasAtomicas)){
		return respuesta;
	}

	if (strstr(ALGORITMO_REEMPLAZO, "CIRC") != NULL) {
		log_info(logger, "Algoritmo usado: Algoritmo Circular");
		respuesta = algoritmoCircular(listaEntradasAtomicas);
	}
	if (strstr(ALGORITMO_REEMPLAZO, "LRU") != NULL) {
		log_info(logger, "Algoritmo usado: Algoritmo LRU");
		respuesta = algoritmoLeastRecentlyUsed(listaEntradasAtomicas);
	}
	if (strstr(ALGORITMO_REEMPLAZO, "BSU") != NULL) {
		log_info(logger, "Algoritmo usado: Algoritmo BSU");
		respuesta = algoritmoBiggestSpaceUsed(listaEntradasAtomicas);
	}

	free(listaEntradasAtomicas);
	return respuesta;
}


int algoritmoLeastRecentlyUsed(t_list* listaFiltradaAtomica) {
	int operaciones_max = 0;
	int resultado_entrada = 0;

	void _buscoMaximo(t_registro_tabla_entrada* registro) {
		char* key = string_itoa(registro->numero_entrada);
		t_registro_diccionario_entrada *  registroDiccionario = dictionary_get(DICCIONARITY_ENTRADA, key);
		if (operaciones_max < registroDiccionario->cant_operaciones) {
			operaciones_max = registroDiccionario->cant_operaciones;
			resultado_entrada = registro->numero_entrada;
		}
	}
	list_iterate(listaFiltradaAtomica,(void*) _buscoMaximo);
	return resultado_entrada;
}


t_list* filtrar_atomico(){
	t_list * lista_filtrada = create_list();
	void _buscoAtomico(t_registro_tabla_entrada* registro) {

		bool _estaCargado(t_registro_tabla_entrada* reg){
			return (strcmp(reg->clave,registro->clave) == 0);
		}

		//me fijo cuantas veces aparece esta clave en la tabla, si es atomico solo aparece una vez
		if(list_count_satisfying(TABLA_ENTRADA,(void*)_estaCargado) == 1){
			list_add(lista_filtrada,registro);
		}
	}
	list_iterate(TABLA_ENTRADA,(void*) _buscoAtomico);


	//print lista filtrada
//	void _printLista(t_registro_tabla_entrada* registro) {
//		char* aux = string_from_format("LISTA ATOMICOS : ENTRADA %d CLAVE %s",registro->numero_entrada,registro->clave);
//		log_info(logger, aux);
//		free(aux);
//	}
//	list_iterate(lista_filtrada,(void*) _printLista);

	return lista_filtrada;
}


int algoritmoBiggestSpaceUsed(t_list* listaFiltradaAtomica) {
	//el que tiene tamanio libre minimo es el mas grande
	int tamanio_libre_min = TAMANIO_ENTRADA;
	int resultado_entrada = 0;

	void _buscoMaxEspacio(t_registro_tabla_entrada* registro) {
		char* key = string_itoa(registro->numero_entrada);
		t_registro_diccionario_entrada *  registroDiccionario = dictionary_get(DICCIONARITY_ENTRADA, key);
		if (tamanio_libre_min > registroDiccionario->tamanio_libre) {
			tamanio_libre_min = registroDiccionario->tamanio_libre;
			resultado_entrada = registro->numero_entrada;
		}
	}
	list_iterate(listaFiltradaAtomica,(void*) _buscoMaxEspacio);
	return resultado_entrada;
}


int algoritmoCircular(t_list* listaFiltradaAtomica) {
	int respuesta = -1;
	int puntero = -1;

	if (PUNTERO_DIRECCION_CIRCULAR < list_size(listaFiltradaAtomica)) {

		puntero = PUNTERO_DIRECCION_CIRCULAR;
		t_registro_tabla_entrada* reg = list_get(listaFiltradaAtomica,puntero);
		respuesta = reg->numero_entrada;

		PUNTERO_DIRECCION_CIRCULAR = PUNTERO_DIRECCION_CIRCULAR + 1;
		return respuesta;
	} else {

		PUNTERO_DIRECCION_CIRCULAR = 0;
		puntero = PUNTERO_DIRECCION_CIRCULAR;

		t_registro_tabla_entrada* reg = list_get(listaFiltradaAtomica,puntero);
		respuesta = reg->numero_entrada;
		return respuesta;//retorna el primero
	}
}

void crearPuntoDeMontaje(char* path){
	DIR* dir = opendir(path);
	if (dir)
	{
		/* Directorio existe */
		closedir(dir);

		return;
	}
	else if (ENOENT == errno)
	{
		/* Directorio no existe */

		mkdir(path, 0777);
	}
}
