
#include "funcionalidad_instancia.h"

void intHandler(int dummy) {
	if (dummy != 0) {
		printf("\nFinalizó con una interrupcion :'(, codigo: %d!!\n", dummy);
		exit(dummy);

	}
}

void get_parametros_config(){
	t_config* config = config_create("config.cfg");
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


	config_destroy(config);
}

void free_parametros_config(){

	free(IP_CONFIG_COORDINADOR);
	free(ALGORITMO_REEMPLAZO);
	free(PUNTO_MONTAJE);
	free(NOMBRE_INSTANCIA);
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
		printf("No se puede enviar el resultado al coordinador\n");
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	printf("Envie mi resultado correctamente\n");
	pthread_mutex_unlock(&MUTEX_INSTANCIA);
}


void recibo_mensaje_aceptacion(int fd_coordinador){
	int numbytes = 0;
	int32_t resultado_aceptacion = 0;
	if ((numbytes = recv(fd_coordinador, &resultado_aceptacion, sizeof(int32_t), 0)) <= 0) {
		if(numbytes == 0){
			printf("Se desconecto el coordinador\n");
		}else{
			printf("No se pudo recibir el resultado de la acpetacion\n");
		}
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	if(resultado_aceptacion == 1){
		//me rechazaron por nombre repetido
		printf("Me rechazaron por nombre repetido\n");
		free_algo_punt_nom();
		exit(1);
	}
	printf("Me aceptaron y encolaron en la lista de instancias :)\n");

}
//recibe la linea, la procesa ... y retorna un valor
int recibo_sentencia(int fd_coordinador){
	int32_t long_clave = 0;
	int32_t long_valor = 0;
	int32_t tipo_operacion = 0;
	int32_t numbytes = 0;
	int32_t numeroEntrada;
	int respuesta = 0;

	if ((numbytes = recv(fd_coordinador, &tipo_operacion, sizeof(int32_t), 0)) <= 0) {
			printf("Coordinador desconectado\n");
			free_algo_punt_nom();
			free_estruct_admin();
			close(fd_coordinador);
			exit(1);
	}
	pthread_mutex_lock(&MUTEX_INSTANCIA);
	/*PROCESO.....*/
	if(tipo_operacion == SET){ //SET CLAVE VALOR

		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño de la clave\n");
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			printf("No se pudo recibir la clave\n");
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		if ((numbytes = recv(fd_coordinador, &long_valor, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño del valor\n");
			free_algo_punt_nom();
			free(clave_recibida);
			close(fd_coordinador);
			exit(1);
		}

		char* valor_recibido = malloc(sizeof(char)*long_valor);
		if ((numbytes = recv(fd_coordinador, valor_recibido, long_valor, 0)) <= 0) {
			if(numbytes == 0){
				printf("Se desconecto el coordinador\n");
			}else{
				printf("No se pudo recibir el valor del la operacion\n");
			}
			free(valor_recibido);
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		printf("Recibi para hacer SET clave: %s valor: %s\n",clave_recibida,valor_recibido);
		respuesta = ejecuto_set(clave_recibida,valor_recibido);
		free(valor_recibido);
		free(clave_recibida);

	}
	if(tipo_operacion == STORE){ //STORE CLAVE
		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño de la clave\n");
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			printf("No se pudo recibir la clave\n");
			free(clave_recibida);
			free_algo_punt_nom();
			close(fd_coordinador);
			exit(1);
		}
		printf("Recibi para hacer STORE clave: %s\n",clave_recibida);

		//respuesta = OK_STORE_INSTANCIA;
		respuesta = ejecuto_store(clave_recibida,0);
		free(clave_recibida);
	}

	return respuesta;
}

int ejecuto_set(char* clave_recibida,char* valor_recibido){

	int entrada_inicial = -1;
	int entradas_necesarias = ( strlen(valor_recibido) + 1 ) / TAMANIO_ENTRADA;
	int cant_espacio_disponibles = espacio_diponible(entradas_necesarias);
	if(cant_espacio_disponibles >= entradas_necesarias){
		if(son_contiguos(entradas_necesarias,&entrada_inicial)){
			guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}else{
			compacto();
			guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}
	}else{
		aplico_reemplazo(entradas_necesarias - cant_espacio_disponibles);//reemplaza tanta cantidad de claves como espacios necesite
		if(son_contiguos(entradas_necesarias,&entrada_inicial)){
			guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}else{
			compacto();
			guardo_valor(entrada_inicial,clave_recibida,valor_recibido,entradas_necesarias);
		}
	}
	return OK_SET_INSTANCIA;
}

void compacto(){
	//aviso al coordinador q compacten todas las instancias
	//acomoda las entradas
}

//reemplaza tantas veces como entradas_necesarias - cant_espacio_disponibles
void aplico_reemplazo(int cant_espacios_buscados){
	int i;
	if(cant_espacios_buscados > 0){
		for(i = 0 ; i < cant_espacios_buscados; i++){
			//numeroEntrada = aplicarAlgoritmoReemplazo(clave_recibida, valor_recibido);
			//libero_entrada(numeroEntrada)
		}

	}
}

void guardo_valor(int entrada_inicial,char* clave_recibida,char* valor_recibido,int entradas_necesarias){
	int i;
	int tamanio_contenido = 0;
	if(entradas_necesarias > 1){
		for(i=0;i<entradas_necesarias;i++){

			tamanio_contenido = strlen(valor_recibido) + 1;

			if(tamanio_contenido > TAMANIO_ENTRADA){
				memcpy(STORAGE[entrada_inicial + i],valor_recibido,TAMANIO_ENTRADA-1);
				STORAGE[entrada_inicial][TAMANIO_ENTRADA-1]='\0';
				memcpy(valor_recibido,valor_recibido + TAMANIO_ENTRADA -1 ,tamanio_contenido);
			}else{
				memcpy(STORAGE[entrada_inicial + i],valor_recibido,tamanio_contenido);
			}
			//actualizo tabla
			cargo_actualizo_tabla(clave_recibida,entrada_inicial+i,tamanio_contenido);
			//actualizo diccionario
			cargo_actualizo_diccionario(entrada_inicial+i,tamanio_contenido);
		}
	}else{
		strcpy(STORAGE[entrada_inicial],valor_recibido);
		//actualizo tabla
		cargo_actualizo_tabla(clave_recibida,entrada_inicial,strlen(valor_recibido) + 1);
		//actualizo diccionario
		cargo_actualizo_diccionario(entrada_inicial, strlen(valor_recibido) + 1);
	}

	//ordeno tabla por numero de entrada(nos sirve para cuando buscamos claves diferentes)
	order_tabla_by(TABLA_ENTRADA,(void*) by_numero_entrada);
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
				return true;
			}
		}else{
			contiguo = 0;
		}
	}
	return false;

}

//me dice cuantos espacios libre hay
int espacio_diponible(int entradas_necesarias){
	int contador = 0;
	void _espacioDisponible(char* _, t_registro_diccionario_entrada* diccionario) {
		if(diccionario->libre){
			contador++;
		}
	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_espacioDisponible);

	return contador;
}

int ejecuto_store(char* clave_recibida,int isDUMp){

	//si la clave-valor ya no existe en la instancia dado que se pizo: FALLO_INSTANCIA_CLAVE_SOBREESCRITA
	char* valor_del_storage = get_valor_by_clave(clave_recibida,isDUMp);
	if(valor_del_storage == NULL){
		printf("ERROR: no existe la clave-valor dado que alguna de su entradas se reemplazo\n");
		return FALLO_INSTANCIA_CLAVE_SOBREESCRITA;
	}

	printf("El valor a guardar es:%s\n",valor_del_storage);

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


char* get_valor_by_clave(char * clave_recibida,int isDUmp){

	char* valor_buscado = malloc(sizeof(char)* TAMANIO_ENTRADA* CANT_ENTRADA);
	strcpy(valor_buscado,"");

	bool _esClave(t_registro_tabla_entrada* entrada) { return strcmp(entrada->clave,clave_recibida)== 0;}
	t_list* tabla_entrada =  list_filter(TABLA_ENTRADA,(void*) _esClave);

	if(tabla_entrada!=NULL){
		void _armoValor(t_registro_tabla_entrada* entrada) {
			strcat(valor_buscado,STORAGE[entrada->numero_entrada]);
			if(!isDUmp){
				//actualizo cant. operacion
				actualizo_cant_operaciones(entrada->numero_entrada);
			}
		}
		list_iterate(tabla_entrada,(void*)_armoValor);
		valor_buscado[strlen(valor_buscado)] = '\0';
		return valor_buscado;
	}
	return NULL;

}

void recibo_datos_entrada(int fd_coordinador){
	int numbytes = 0;
	if ((numbytes = recv(fd_coordinador, &TAMANIO_ENTRADA, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir el tamaño de la entrada\n");
		free_algo_punt_nom();
		exit(1);
	}
	if ((numbytes = recv(fd_coordinador, &CANT_ENTRADA, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la cantidad de entradas\n");
		free_algo_punt_nom();
		exit(1);
	}
	printf("Recibi tamaño de entrada: %d y cantidad de entrada: %d correctamente\n",TAMANIO_ENTRADA,CANT_ENTRADA);

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
		printf("No pude enviar mis datos al coordinador\n");
		free(bufferEnvio);
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	printf("Envie mi nombre y tamaño libre:%d correctamente\n",espacio_libre);
	free(bufferEnvio);
}

int obtener_espacio_libre(){
	int espacio_total_disponible = 0;

	void _buscaEspacioLibre(char* _, t_registro_diccionario_entrada* registro) {
		espacio_total_disponible = espacio_total_disponible + registro->tamanio_libre;
	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_buscaEspacioLibre);
	return espacio_total_disponible;
}

//con los datos de entrada que recibi, puedo inicializar mis estructuras
void inicializo_estructuras(){

	//inicializo diccionario de entrada
	DICCIONARITY_ENTRADA = create_diccionarity();

	//inicializo storage
	STORAGE = malloc(sizeof(char*)* CANT_ENTRADA);
	int i = 0;
	for(i= 0;i<CANT_ENTRADA;i++){
		STORAGE[i] = malloc(sizeof(char) * TAMANIO_ENTRADA); //es 100, osea 99 caractes posta
		strcpy(STORAGE[i],"");

		t_registro_diccionario_entrada* registro = get_new_registro_dic_entrada(1,0,TAMANIO_ENTRADA);
		dictionary_put(DICCIONARITY_ENTRADA,string_itoa(i),registro);
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
	printf("Leo mis archivos previamente guardados si es que tengo..\n");
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
		printf("Error al tratar de abrir la carpeta\n" );
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
		printf("Tenemos para reestablecer el archivo :%s\n", nombre_archivo_sin_salto);
		contador_archivos ++;
		reestablesco_archivo(nombre_archivo_sin_salto);
	}
	if(contador_archivos == 0){
		printf("No hay nada para reestablecer\n");
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
		printf("Error al tratar de usar mmap!\n");
		free(path_archivo);
		free_algo_punt_nom();
		free_estruct_admin();
		close(fd);
		exit(1);
	}
	int32_t len_clave = strlen(nombre_archivo) - 4;
	char* clave = malloc(len_clave + 1);
	memcpy(clave,nombre_archivo,len_clave); //copio sin el .txt
	clave[len_clave] = '\0';


	int32_t len_valor = tamanio_contenido + 1;
	char* valor = malloc(len_valor);
	memcpy(valor,mmappedData,len_valor);
	valor[len_valor] = '\0';


	if (munmap(mmappedData, tamanio_contenido) == -1){
		printf("Error al tratar de liberar memoria de un mmap!\n");
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

	close(fd);
	free(clave);
	free(valor);
	free(path_archivo);
}

void cargar_estructuras(char* clave,char* valor,int tamanio_contenido){
	int i = 0;
	for(i = 0; i < CANT_ENTRADA; i++){
		if(strcmp(STORAGE[i],"")== 0){
			//esta vacio
			if(tamanio_contenido <= TAMANIO_ENTRADA){
				//entra en uno solo
				strcpy(STORAGE[i],valor);
				printf("Cargo en la entrada numero: %d el valor: %s\n",i,valor);

				//actualizo tabla
				cargo_actualizo_tabla(clave,i,tamanio_contenido);
				//actualizo diccionario
				cargo_actualizo_diccionario(i,tamanio_contenido);

				break;
			}else{
				//no entra en uno solo, entoces los divido
				memcpy(STORAGE[i],valor,TAMANIO_ENTRADA-1);
				STORAGE[i][TAMANIO_ENTRADA-1]='\0';
				printf("Cargo en la entrada numero: %d el valor: %s\n",i,STORAGE[i]);

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
		printf("Actualizo en mi tabla la entrada:%d clave:%s tamaño del valor:%d\n",numero_entrada,clave,tamanio_contenido);
	}else{
		//si no esta guardado, lo guardo por primera vez
		t_registro_tabla_entrada * nuevo_registro = get_new_registro_tabla_entrada(numero_entrada,clave,tamanio_contenido);
		list_add(TABLA_ENTRADA,nuevo_registro);
		printf("Cargo en mi tabla la entrada:%d clave:%s tamaño del valor:%d\n",numero_entrada,clave,tamanio_contenido);
	}
}

void cargo_actualizo_diccionario(int numero_entrada,int tamanio_contenido){
	char * key = string_itoa(numero_entrada);
	//buscamos si ya esta esa entrada guardada
	if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
		//existe la key en el diccionario
		t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		actualizo_cant_operaciones(numero_entrada);
		registro_diccionario->libre = 0;
		registro_diccionario->tamanio_libre = TAMANIO_ENTRADA - tamanio_contenido;
		printf("Actualizo en mi diccionario la entrada:%d-ocupada-operaciones:%d-tamaño libre:%d\n",numero_entrada,registro_diccionario->cant_operaciones,registro_diccionario->tamanio_libre);
	}else{
		//no existe, lo crea
		t_registro_diccionario_entrada * registro_diccionario = get_new_registro_dic_entrada(0,0,(TAMANIO_ENTRADA - tamanio_contenido));
		dictionary_put(DICCIONARITY_ENTRADA,key,registro_diccionario);
		actualizo_cant_operaciones(numero_entrada);
		printf("Cargo en mi diccionario la entrada:%d-ocupada-operaciones:1-tamaño libre:%d\n",numero_entrada,registro_diccionario->tamanio_libre);
	}

}

//a la entrada del numero seteo operaciones en 0 y a las ademas entradas OCUPADAS las operaciones las sumo en +1
void actualizo_cant_operaciones(int numero_entrada){
	char * key = string_itoa(numero_entrada);
	//buscamos si ya esta esa entrada
	if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
		//existe la key en el diccionario
		t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		registro_diccionario->cant_operaciones = 0; //porque fue usada recien
	}

	//a las otras claves OCUPADAS les tengo q sumar 1 la cant de operaciones
	void _cambioCantOperaciones(char* key_registro, t_registro_diccionario_entrada* diccionario) {
		if((strcmp(key_registro,key)!=0) && (diccionario->libre == 0)){
			diccionario->cant_operaciones++;
		}
	}
	dictionary_iterator(DICCIONARITY_ENTRADA,(void*)_cambioCantOperaciones);
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
		printf("Empieza dump....\n");

		t_list* tabla_solo_claves = get_only_clave();

		void _aplicaSTORE(t_registro_tabla_entrada* una_entrada) {
			int resultado = ejecuto_store(una_entrada->clave,1);
			if(resultado == FALLO_INSTANCIA_CLAVE_SOBREESCRITA){
				printf("Fallo al hacer DUMP de la clave: %s\n",una_entrada->clave);
			}
			printf("DUMP de la clave: %s correctamente hecho\n",una_entrada->clave);

		}
		list_iterate(tabla_solo_claves,(void*)_aplicaSTORE);
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


int aplicarAlgoritmoReemplazo(char* clave_recibida, char* valor_recibido){

	if (strstr(ALGORITMO_REEMPLAZO, "C") != NULL) {

		}
	if (strstr(ALGORITMO_REEMPLAZO, "LRU") != NULL) {
			printf("INFO: Algoritmo LRU\n");

	}
	if (strstr(ALGORITMO_REEMPLAZO, "BSU") != NULL) {
			printf("INFO: Algoritmo BSU\n");

	}
	return -1;
}
