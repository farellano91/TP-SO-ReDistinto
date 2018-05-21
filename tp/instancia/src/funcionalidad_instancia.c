
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

void envio_resultado_al_coordinador(int sockfd,int resultado){

	if(send(sockfd, &resultado, sizeof(int32_t), 0) == -1) {
		printf("No se puede enviar el resultado al coordinador\n");
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	printf("Envie mi resultado correctamente\n");
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
	int respuesta = 0;

	if ((numbytes = recv(fd_coordinador, &tipo_operacion, sizeof(int32_t), 0)) <= 0) {
			printf("Coordinador desconectado\n");
			free_algo_punt_nom();
			free_estruct_admin();
			exit(1);
	}

	/*PROCESO.....*/
	if(tipo_operacion == SET){ //SET CLAVE VALOR
		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño de la clave\n");
			free_algo_punt_nom();
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			printf("No se pudo recibir la clave\n");
			free(clave_recibida);
			free_algo_punt_nom();
			exit(1);
		}

		if ((numbytes = recv(fd_coordinador, &long_valor, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño del valor\n");
			free_algo_punt_nom();
			free(clave_recibida);
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
			exit(1);
		}

		printf("Recibi para hacer SET clave: %s valor: %s\n",clave_recibida,valor_recibido);
		//TODO:-----> proceso_operacion(tipo_operacion,clave_recibida,valor_recibido);

		//por ahora hardcodeamos que la operacion salio bien
		respuesta = OK_SET_INSTANCIA;
		free(clave_recibida);
		free(valor_recibido);
	}
	if(tipo_operacion == STORE){ //STORE CLAVE
		if ((numbytes = recv(fd_coordinador, &long_clave, sizeof(int32_t), 0)) <= 0) {
			printf("No se pudo recibir le tamaño de la clave\n");
			free_algo_punt_nom();
			exit(1);
		}

		char* clave_recibida = malloc(sizeof(char)*long_clave);
		if ((numbytes = recv(fd_coordinador, clave_recibida, long_clave, 0)) <= 0) {
			printf("No se pudo recibir la clave\n");
			free(clave_recibida);
			free_algo_punt_nom();
			exit(1);
		}
		printf("Recibi para hacer STORE clave: %s\n",clave_recibida);
		//TODO:-----> proceso_operacion(tipo_operacion,clave_recibida,valor_recibido);

		//por ahora hardcodeamos que la operacion salio bien
		respuesta = OK_STORE_INSTANCIA;
		free(clave_recibida);
	}

	return respuesta;
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
	//
	int espacio_libre = obtener_espacio_libre();
	printf("Deberia avisar que tengo %d de espacio libre solo\n",espacio_libre);

	int32_t longitud_mensaje = strlen(NOMBRE_INSTANCIA) + 1;
	void* bufferEnvio = malloc(sizeof(int32_t)+ sizeof(char)*longitud_mensaje);
	memcpy(bufferEnvio, &longitud_mensaje,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),NOMBRE_INSTANCIA,longitud_mensaje);

	if (send(fd_coordinador, bufferEnvio,sizeof(int32_t)+ sizeof(char)*longitud_mensaje, 0) == -1) {
		printf("No pude enviar mis datos al coordinador\n");
		free(bufferEnvio);
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}
	printf("Envie mi nombre correctamente\n");
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
//	char nombre_archivo[100];
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

	size_t tamanio_contenido = getFilesize(path_archivo); //tamaño completo, osea "asdasd\n" => 7

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

	int32_t len_valor = tamanio_contenido - 1;
	char* valor = malloc(len_valor + 1);
	memcpy(valor,mmappedData,len_valor);//el dato dentro del archivo (osea el valor) tambien viene con \n
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
	cargar_estructuras(clave,valor,len_valor + 1);

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
				printf("Cargo en la entrada numero: %d el valor: %s\n",i+1,valor);

				//actualizo tabla
				cargo_actualizo_tabla(clave,i,tamanio_contenido);
				//actualizo diccionario
				cargo_actualizo_diccionario(i,tamanio_contenido);

				break;
			}else{
				//no entra en uno solo, entoces los divido
				memcpy(STORAGE[i],valor,TAMANIO_ENTRADA-1);
				STORAGE[i][TAMANIO_ENTRADA]='\0';
				printf("Cargo en la entrada numero: %d el valor: %s\n",i+1,valor);

				memcpy(valor,valor + TAMANIO_ENTRADA,tamanio_contenido);
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
		printf("Actualizo en mi tabla la entrada:%d clave:%s tamaño del valor:%d\n",numero_entrada+1,clave,tamanio_contenido);
	}else{
		//si no esta guardado, lo guardo por primera vez
		t_registro_tabla_entrada * nuevo_registro = get_new_registro_tabla_entrada(numero_entrada,clave,tamanio_contenido);
		list_add(TABLA_ENTRADA,nuevo_registro);
		printf("Cargo en mi tabla la entrada:%d clave:%s tamaño del valor:%d\n",numero_entrada+1,clave,tamanio_contenido);
	}
}

void cargo_actualizo_diccionario(int numero_entrada,int tamanio_contenido){
	char * key = string_itoa(numero_entrada);
	//buscamos si ya esta esa entrada guardada
	if(dictionary_has_key(DICCIONARITY_ENTRADA,key)){
		//existe la key en el diccionario
		t_registro_diccionario_entrada * registro_diccionario = dictionary_get(DICCIONARITY_ENTRADA,key);
		registro_diccionario->cant_operaciones++;
		registro_diccionario->libre = 0;
		registro_diccionario->tamanio_libre = TAMANIO_ENTRADA - tamanio_contenido;
		printf("Actualizo en mi diccionario la entrada:%d-ocupada-operaciones:%d-tamaño libre:%d\n",numero_entrada,registro_diccionario->cant_operaciones,registro_diccionario->tamanio_libre);
	}else{
		//no existe, lo crea
		t_registro_diccionario_entrada * registro_diccionario = get_new_registro_dic_entrada(0,1,(TAMANIO_ENTRADA - tamanio_contenido));
		dictionary_put(DICCIONARITY_ENTRADA,key,registro_diccionario);
		printf("Cargo en mi diccionario la entrada:%d-ocupada-operaciones:1-tamaño libre:%d\n",numero_entrada,registro_diccionario->tamanio_libre);
	}

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
