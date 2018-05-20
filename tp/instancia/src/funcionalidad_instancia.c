
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

	ALGORITMO_REEMPLAZO = malloc(sizeof(char) * 100);
	strcpy(ALGORITMO_REEMPLAZO,config_get_string_value(config, "ALGORITMO_REEMPLAZO"));

	PUNTO_MONTAJE = malloc(sizeof(char) * 100);
	strcpy(PUNTO_MONTAJE,config_get_string_value(config, "PUNTO_MONTAJE"));

	NOMBRE_INSTANCIA = malloc(sizeof(char) * 100);
	strcpy(NOMBRE_INSTANCIA,config_get_string_value(config, "NOMBRE_INSTANCIA"));


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
	//revisar
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

//con los datos de entrada que recibi, puedo inicializar mis estructuras
void inicializo_estructuras(){

	//inicializo storage
	STORAGE = malloc(sizeof(char*)* CANT_ENTRADA);
	int i = 0;
	for(i= 0;i<CANT_ENTRADA;i++){
		STORAGE[0] = malloc(sizeof(char) * TAMANIO_ENTRADA);
	}

	//inicializo tabla entradas
	TABLA_ENTRADA = create_list();

	//inicializo diccionario de entrada
	DICCIONARITY_ENTRADA = create_diccionarity();
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

	size_t tamanio_contenido = getFilesize(path_archivo);

	int fd = open(path_archivo, O_RDONLY, 0);
	void* mmappedData = mmap(NULL, tamanio_contenido, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
	if ( mmappedData == MAP_FAILED ){
		printf("Error al tratar de usar mmap!\n");
		free(path_archivo);
		free_algo_punt_nom();
		free_estruct_admin();
		exit(1);
	}

	char* clave = malloc(sizeof(char)*100);
	memcpy(clave,nombre_archivo,strlen(nombre_archivo)-4); //copio sin el .txt

	char* valor = malloc(sizeof(char)*1024);
	memcpy(valor,mmappedData,tamanio_contenido-1);//el dato dentro del archivo (osea el valor) tambien viene con \n

	int rc = munmap(mmappedData, tamanio_contenido);

	printf("Clave: %s Valor: %s de tamaño: %d\n",clave,valor,tamanio_contenido);

	//cargo mis estructuras
	cargar_estructuras(clave,valor,tamanio_contenido);

	free(clave);
	free(valor);
	free(path_archivo);
}

void cargar_estructuras(char* clave,char* valor,int tamanio_contenido){
	//inserto en el diccionario de entradas

	//inserto en el storage

	//inserto en la tabla de entradas
}

t_dictionary* create_diccionarity(){
	t_dictionary * diccionarity = dictionary_create();
	return diccionarity;
}

t_list* create_list(){
	t_list * list = list_create();
	return list;
}
