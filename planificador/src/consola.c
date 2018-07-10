#include "consola.h"

typedef int Function ();
typedef char **CPPFunction ();

/* The names of functions that actually do the manipulation. */
int com_pausa (), com_continuar (), com_bloquear (), com_desbloquear (), com_listar ();
int com_kill (), com_status (), com_deadlock (), com_quit();

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  char *name;			/* User printable name of the function. */
//  Function *func;     /* Function to call to do the job. */
  int (*func)();
  char *doc;			/* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
  { "pausa", com_pausa, "Pausar al planificador" },
  { "continuar", com_continuar, "Planificador puede continuar" },
  { "bloquear", com_bloquear, "Bloquea por clave y ID" },
  { "desbloquear", com_desbloquear, "Desbloquear por clave y ID" },
  { "listar", com_listar, "Listar ESI bloqueados para el recurso" },
  { "kill", com_kill, "Finaliza proceso por ID" },
  { "status", com_status, "Lista info sobre las instancias" },
  { "deadlock", com_deadlock, "Activa deadlock" },
  { "quit", com_quit, "Quit using Fileman" },
  { (char *)NULL, NULL, (char *)NULL }
};

/* Forward declarations. */
char *stripwhite ();
COMMAND *find_command ();

/* The name of this program, as taken from argv[0]. */
char *progname;

/* When non-zero, this global means the user is done using this program. */
int done;

/* Execute a command line. */
int execute_line (char *line){
  register int i;
  COMMAND *command;
  char *word;

  /* Isolate the command word. */
  i = 0;
  while (line[i] && whitespace (line[i]))
    i++;
  word = line + i;

  while (line[i] && !whitespace (line[i]))
    i++;

  if (line[i])
    line[i++] = '\0';

  command = find_command (word);

  if (!command)
    {
      fprintf (stderr, "Comando no reconocido.\n");
      return (-1);
    }

  /* Get argument to command, if any. */
  while (whitespace (line[i]))
    i++;

  word = line + i;

  /* Call the function. */
  return ((*(command->func)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND * find_command (char *name){
  register int i;

  for (i = 0; commands[i].name; i++)
    if (strcmp (name, commands[i].name) == 0)
      return (&commands[i]);

  return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char * stripwhite (char *string){
  register char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;

  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

int com_pausa (char *arg){
	if(control_parametros(arg,0)){
		return 0;
	}
	puts("Comando pausa!!");
	pthread_mutex_lock(&MUTEX);
	PLANIFICADOR_EN_PAUSA = true;
	pthread_mutex_unlock(&MUTEX);
	return (0);
}

int com_continuar (char *arg){
	if(control_parametros(arg,0)){
		return 0;
	}
	puts("Comando continuar!!");
	pthread_mutex_lock(&MUTEX);
	PLANIFICADOR_EN_PAUSA = false;
	pthread_mutex_unlock(&MUTEX);

	pthread_cond_signal(&CONDICION_PAUSA_PLANIFICADOR);

	//Nuevo: continua flujo si esta disponible el cpu (esto es si el flujo quedo frenado)
	pthread_mutex_lock(&MUTEX);
	pthread_mutex_lock(&READY);
	pthread_mutex_lock(&EXECUTE);

	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE,list_get(LIST_READY, 0));

			t_Esi* unEsi = list_get(LIST_EXECUTE, 0);
			if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
				double rr = get_prioridad_HRRN(unEsi);
				printf("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f\n",unEsi->id,unEsi->tiempoEnListo,unEsi->estimacion,rr);
			}

			list_remove(LIST_READY, 0);
			pthread_mutex_unlock(&EXECUTE);
			continuar_comunicacion();
			pthread_mutex_unlock(&READY);
			pthread_mutex_unlock(&MUTEX);
			return 0;

	}
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&READY);
	pthread_mutex_unlock(&MUTEX);
	//---------------------------------------------

	return (0);
}

int control_parametros(char* arg,int cant_parametros){
	char** split = string_split(arg, " ");
	int contador = 0;
	while (split[contador] != NULL) {
		contador++;
	}
	if (contador != cant_parametros) {
		printf("Error: En la cantidad de parametros, debe ingresar: %d parametros\n",cant_parametros);
		return (1);
	}
	destroySplit(split);
	return 0;
}

void destroySplit(char** split){
	int i = 0;
	while (split[i] != NULL) {
		free(split[i]);
		i++;
	}

	free(split);
}

int com_bloquear (char *arg){
  if(control_parametros(arg,2)){
	return 0;
  }
  char** split = string_split(arg, " ");
  char* clave = string_duplicate(split[0]);
  int esi_id = atoi(split[1]);
  t_Esi* esi_ready;
  t_Esi* esi_ejecucion;
  t_nodoBloqueado* esi_bloqueado;
  pthread_mutex_lock(&EXECUTE);
  pthread_mutex_lock(&READY);
  pthread_mutex_lock(&BLOCKED);
  pthread_mutex_lock(&ESISBLOQUEADOR);

	bool _esElid(t_Esi* un_esi) {
		return un_esi->id == esi_id;
	}

	bool _esElidBloqueado(t_nodoBloqueado* un_nodo) {
		return un_nodo->esi->id == esi_id;
	}

	bool _es_la_clave(t_esiBloqueador* esi) {
		return string_equals_ignore_case(esi->clave, clave);
	}

	  esi_ready = (t_Esi*)list_find(LIST_READY, (void*)_esElid);
	  esi_ejecucion = (t_Esi*)list_find(LIST_EXECUTE, (void*)_esElid);
	  esi_bloqueado = (t_nodoBloqueado*)list_find(LIST_BLOCKED, (void*)_esElidBloqueado);

	  if(esi_bloqueado != NULL){
		  printf("El ESI con id = %d ya fue bloqueado por la clave: %s\n", esi_bloqueado->esi->id, esi_bloqueado->clave);

	  }

	  else if(list_any_satisfy(LIST_ESI_BLOQUEADOR, (void*)_es_la_clave)){

	  if(esi_ready != NULL){
		 t_nodoBloqueado* esi_bloqueado = get_nodo_bloqueado(esi_ready,clave);
		 list_remove_by_condition(LIST_READY, (void*)_esElid);
		 list_add(LIST_BLOCKED,esi_bloqueado);
		 printf("El ESI con id = %d pasó de Listo a Bloqueado por la clave %s\n", esi_id, clave);
	  }
	  else if(esi_ejecucion != NULL){
		  esi_ejecucion->status = 4;
		  CLAVE_BLOQUEO_CONSOLA = clave;
		  printf("El ESI con id = %d pasó de Ejecutando a Bloqueado por la clave %s\n", esi_id, clave);

	  }else{
		  printf("La clave %s existe en el sistema pero el ESI con id = %d no. El bloqueo ha fallado.\n", clave, esi_id);
	  }

	  }else{
		  if(esi_ready != NULL){
			  t_esiBloqueador* esi_bloqueador = get_esi_bloqueador(esi_ready, clave);
			  list_add(LIST_ESI_BLOQUEADOR,esi_bloqueador);
			  printf("El ESI con id = %d tomó la clave %s\n", esi_id, clave);
		  }else if(esi_ejecucion != NULL){
			     t_esiBloqueador* esi_bloqueador = get_esi_bloqueador(esi_ejecucion, clave);
				 list_add(LIST_ESI_BLOQUEADOR,esi_bloqueador);
				 printf("El ESI con id = %d tomó la clave %s\n", esi_id, clave);
		  }else{
				t_Esi* un_esi = malloc(sizeof(t_Esi));
				un_esi->id = 0;
				un_esi->fd = 0;
				t_esiBloqueador* esiBLoqueador = get_esi_bloqueador(un_esi, clave);
				list_add(LIST_ESI_BLOQUEADOR,esiBLoqueador);
				printf("La clave %s fue agregada como clave bloqueada por config\n", clave);

		  }
	  }
	  pthread_mutex_unlock(&ESISBLOQUEADOR);
	  pthread_mutex_unlock(&BLOCKED);
	  pthread_mutex_unlock(&READY);
	  pthread_mutex_unlock(&EXECUTE);
	  destroySplit(split);
  return (0);
}

int com_desbloquear (char *arg){
	if(control_parametros(arg,1)){
		return 0;
	}
	puts("Comando desbloquear!!");
	char* clave = arg;


	//controlo si hay algun esi bloqueado por esta clave
	bool _esElidClave(t_nodoBloqueado* esiBloqueado) { return (strcmp(esiBloqueado->clave,clave)==0);}
	if(!list_is_empty(LIST_BLOCKED) &&
			list_find(LIST_BLOCKED, (void*)_esElidClave) != NULL){
		//Hay al menos 1 esi bloqueado, sacamos solo el primero
		move_esi_from_bloqueado_to_listo(clave);
		//continua flujo si esta disponible el cpu
		pthread_mutex_lock(&MUTEX);
		pthread_mutex_lock(&READY);
		pthread_mutex_lock(&EXECUTE);

		if(!PLANIFICADOR_EN_PAUSA){
			if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
					list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
					list_remove(LIST_READY, 0);

					t_Esi* unEsi = list_get(LIST_EXECUTE, 0);
					if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
						double rr = get_prioridad_HRRN(unEsi);
						printf("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f\n",unEsi->id,unEsi->tiempoEnListo,unEsi->estimacion,rr);
					}
					pthread_mutex_unlock(&EXECUTE);
					continuar_comunicacion();
					pthread_mutex_unlock(&READY);
					pthread_mutex_unlock(&MUTEX);
					return 0;

			}
		}
		pthread_mutex_unlock(&EXECUTE);
		pthread_mutex_unlock(&READY);
		pthread_mutex_unlock(&MUTEX);
	}else{
		pthread_mutex_lock(&ESISBLOQUEADOR);
		//No hay ningun esi bloqueado por esa clave, libero la clave
		bool _esElidClaveB(t_esiBloqueador* esi_bloqueador) { return (strcmp(esi_bloqueador->clave,clave)==0);}
		if(!list_is_empty(LIST_ESI_BLOQUEADOR) &&
				list_find(LIST_ESI_BLOQUEADOR, (void*)_esElidClaveB) != NULL){
			//Solo lo saco de la lista
			list_remove_by_condition(LIST_ESI_BLOQUEADOR,(void*)_esElidClave);
			printf("Libero la clave:%s\n",clave);
		}else{
			printf("No hay clave para liberar\n");
		}
		pthread_mutex_unlock(&ESISBLOQUEADOR);
	}


	/*Antes solo sacaba al primer esi bloqueado y liberaba la clave SIEMPRE
	bool _esElidClave(t_esiBloqueador* esi_bloqueador) { return (strcmp(esi_bloqueador->clave,clave)==0);}
	if(!list_is_empty(LIST_ESI_BLOQUEADOR) &&
			list_find(LIST_ESI_BLOQUEADOR, (void*)_esElidClave) != NULL){
		//Solo lo saco de la lista
		list_remove_by_condition(LIST_ESI_BLOQUEADOR,(void*)_esElidClave);
		printf("Libero la clave:%s\n",clave);
	}else{
		printf("No hay clave para liberar\n");
	}

	move_esi_from_bloqueado_to_listo(clave);
	//continua flujo si esta disponible el cpu
	pthread_mutex_lock(&MUTEX);
	pthread_mutex_lock(&READY);
	pthread_mutex_lock(&EXECUTE);

	if(!PLANIFICADOR_EN_PAUSA){
		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
				list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
				list_remove(LIST_READY, 0);
				pthread_mutex_unlock(&EXECUTE);
				continuar_comunicacion();
				pthread_mutex_unlock(&READY);
				pthread_mutex_unlock(&MUTEX);
				return 0;

		}
	}
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&READY);
	pthread_mutex_unlock(&MUTEX);
	*/

	return (0);
}

int com_listar (char *arg){
	if(control_parametros(arg,1)){
		return 0;
	}
	void _siEsLaClaveMostrarId(t_nodoBloqueado* nodo){
		if(string_equals_ignore_case(nodo->clave, arg)){
			printf("ESI id = %d\n", nodo->esi->id);
		}
	}

	bool _estaLaClaveBloqueada(t_nodoBloqueado* nodo){
		return (string_equals_ignore_case(nodo->clave, arg));
	}

	pthread_mutex_lock(&BLOCKED);

	if(list_any_satisfy(LIST_BLOCKED, (void*) _estaLaClaveBloqueada)){
		printf("Los ESIs bloqueados por la clave: %s son:\n", arg);
		list_iterate(LIST_BLOCKED, (void*) _siEsLaClaveMostrarId);
	}else{
		printf("No hay ningun ESI bloqueado para esa clave\n");
	}

	pthread_mutex_unlock(&BLOCKED);

	return (0);
}


int com_kill(char *arg) {
	if(control_parametros(arg,1)){
		return 0;
	}
	int id_a_borrar = atoi(arg);

	t_Esi* esi_a_borrar;

	t_nodoBloqueado* esi_bloqueado_a_borrar;

	bool _es_el_id_a_borrar(t_Esi* un_esi) {
		return un_esi->id == id_a_borrar;
	}

	bool _es_el_id_a_borrar_bloqueado(t_nodoBloqueado* un_esi_bloqueado) {
		return un_esi_bloqueado->esi->id == id_a_borrar;
	}

	pthread_mutex_lock(&READY);
	pthread_mutex_lock(&BLOCKED);
	pthread_mutex_lock(&EXECUTE);

	if((esi_a_borrar = list_find(LIST_EXECUTE, (void*) _es_el_id_a_borrar)) != NULL) {

		esi_a_borrar->status = 3;

		pthread_mutex_unlock(&EXECUTE);
		pthread_mutex_unlock(&BLOCKED);
		pthread_mutex_unlock(&READY);

		printf("El ESI con id = %d fue eliminado\n", id_a_borrar);

		return 0;

	}else if((esi_a_borrar = list_find(LIST_READY, (void*) _es_el_id_a_borrar)) != NULL) {

           pthread_mutex_unlock(&EXECUTE);
           pthread_mutex_unlock(&BLOCKED);
		   pthread_mutex_unlock(&READY);

           move_esi_from_ready_to_finished(esi_a_borrar->id);
		   free_recurso(esi_a_borrar->fd);

		   pthread_mutex_lock(&SOCKETS);
           list_add(LIST_SOCKETS,(void*)esi_a_borrar->fd);
		   pthread_mutex_unlock(&SOCKETS);

		   printf("El ESI con id = %d fue eliminado\n", id_a_borrar);

		   return 0;

	}else if((esi_bloqueado_a_borrar = list_find(LIST_BLOCKED, (void*) _es_el_id_a_borrar_bloqueado)) != NULL){

		list_remove_by_condition(LIST_BLOCKED,(void*) _es_el_id_a_borrar_bloqueado);

		pthread_mutex_lock(&FINISHED);
		agregar_en_Lista(LIST_FINISHED, esi_bloqueado_a_borrar->esi);
		pthread_mutex_unlock(&FINISHED);

		pthread_mutex_unlock(&BLOCKED);
		pthread_mutex_unlock(&READY);

		int fd = esi_bloqueado_a_borrar->esi->fd;
		free_recurso(esi_bloqueado_a_borrar->esi->fd);
		free_nodoBLoqueado(esi_bloqueado_a_borrar);

		pthread_mutex_lock(&READY);

		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {

			    list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
				list_remove(LIST_READY, 0);

				t_Esi* unEsi = list_get(LIST_EXECUTE, 0);
				if(strcmp(ALGORITMO_PLANIFICACION, "HRRN") == 0){
					double rr = get_prioridad_HRRN(unEsi);
					printf("El PROX. ESI %d a ejecutar tiene tiempo: %d, estimacion: %f RR = %f\n",unEsi->id,unEsi->tiempoEnListo,unEsi->estimacion,rr);
				}

				pthread_mutex_unlock(&EXECUTE);
			    continuar_comunicacion();
		}

		pthread_mutex_unlock(&READY);
		pthread_mutex_unlock(&EXECUTE);

		pthread_mutex_lock(&SOCKETS);
        list_add(LIST_SOCKETS,(void*)fd);
		pthread_mutex_unlock(&SOCKETS);

		printf("El ESI con id = %d fue eliminado\n", id_a_borrar);

		return 0;

	}else{
		printf("El ESI con id = %d no se encuentra en el sistema\n", id_a_borrar);
	}

	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&BLOCKED);
	pthread_mutex_unlock(&READY);

	return (0);
}

int com_status (char *arg){
	if(control_parametros(arg,1)){
		return 0;
	}
	puts("Comando status!!");
	char* clave = arg;
	int32_t longitud_clave = strlen(clave) + 1;

	//si es clave del sistema solo lo informo
	bool _esDelSistema(t_esiBloqueador* esi_bloqueador) { return ((strcmp(esi_bloqueador->clave,clave)==0) && (esi_bloqueador->esi->id == 0));}
	pthread_mutex_lock(&ESISBLOQUEADOR);
	if(!list_is_empty(LIST_ESI_BLOQUEADOR) &&
			list_find(LIST_ESI_BLOQUEADOR, (void*)_esDelSistema) != NULL){
		printf("Valor: Es clave del sistema\n");
		printf("Instancia: Es clave del sistema\n");
		com_listar(clave);
		pthread_mutex_unlock(&ESISBLOQUEADOR);
		return (0);
	}
	pthread_mutex_unlock(&ESISBLOQUEADOR);

	void* bufferEnvio = malloc(sizeof(int32_t) + longitud_clave);
	memcpy(bufferEnvio, &longitud_clave,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),clave,longitud_clave);

	if( FD_COORDINADOR_STATUS == -1 ){
		printf("No tengo el FD del coordinador status\n");
		free(bufferEnvio);
		return (0);
	}
	if (send(FD_COORDINADOR_STATUS, bufferEnvio,sizeof(int32_t) + longitud_clave, 0) == -1) {
		printf("No se pudo enviar pedido status\n");
		free(bufferEnvio);
		return (0);
	}
	free(bufferEnvio);
	int numbytes = 0;
	int32_t respuesta = 0;
	if ((numbytes = recv(FD_COORDINADOR_STATUS, &respuesta,sizeof(int32_t), 0)) <= 0) {
		printf("No se pudo recibir resultado de pedido status\n");
		return (0);
	}else{
		char* nombre_instancia;
		char* valor;
		switch (respuesta) {
			case 1://1: no existe clave en el sistema,
				printf("Valor: No existe la clave en el sistema del coordinador\n");
				printf("Instancia: No existe la clave en el sistema del coordinador\n");
				break;
			case 2://2: existia la clave en una instancia, pero al pedir el valor vemos que la instancia se desconecto,
				printf("Valor: No tenemos el valor dado que la instancia se desconecto\n");
				nombre_instancia = recibo_instancia();
				printf("Instancia: %s\n",nombre_instancia);
				break;
			case 3://3: tiene valor y esta un una instacia conociada
				nombre_instancia = recibo_instancia();
				valor = recibo_valor();
				printf("Valor: %s\n",valor);
				printf("Instancia: %s\n",nombre_instancia);
				free(nombre_instancia);
				free(valor);
				break;
			case 4://4: la instancia es detectada con el algoritmo (como es nueva no tiene valor)
				nombre_instancia = recibo_instancia();
				printf("Valor: No tenemos el valor aun \n");
				printf("Instancia elegida por algoritmo: %s\n",nombre_instancia);
				free(nombre_instancia);
				break;
			case 5://5: la instancia es detectada con el algoritmo es null, osea no hay, y ademas (como es nueva no tiene valor)
				printf("Valor: No tenemos el valor\n");
				printf("Instancia elegida por algoritmo: No tengo instancias para mandar este pedido \n");

				break;
			default:
				break;
		}
		com_listar(clave);
	}
	return (0);
}

char* recibo_instancia(){
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(FD_COORDINADOR_STATUS, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del nombre de la instancia para el status\n");
	}
	char* instancia = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(FD_COORDINADOR_STATUS, instancia, longitud, 0)) == -1) {
		printf("No se pudo recibir nombre de la instancia para el status\n");

	}
	return instancia;
}

char* recibo_valor(){
	int numbytes = 0;
	int32_t longitud = 0;
	if ((numbytes = recv(FD_COORDINADOR_STATUS, &longitud, sizeof(int32_t), 0)) == -1) {
		printf("No se pudo recibir la longitud del valor para el status\n");
	}
	char* valor = malloc(sizeof(char) * longitud);
	if ((numbytes = recv(FD_COORDINADOR_STATUS, valor, longitud, 0)) == -1) {
		printf("No se pudo recibir valor para el status\n");
	}
	return valor;
}

int com_deadlock (char *arg){
	if(control_parametros(arg,0)){
		return 0;
	}
	void _imprimir_id(t_nodoBloqueado* nodo){
		printf("ESI id = %d\n", nodo->esi->id);
	}

	pthread_mutex_lock(&BLOCKED);
	pthread_mutex_lock(&ESISBLOQUEADOR);
    t_list* lista = buscar_deadlock(LIST_ESI_BLOQUEADOR, LIST_BLOCKED);

    if(lista != NULL && list_size(lista) > 0){
       puts("Deadlock detectado. Los ESIs implicados son:");
       list_iterate(lista, (void*)_imprimir_id);
    }else{
    	puts("No se ha detectado un deadlock");
    }
    pthread_mutex_unlock(&ESISBLOQUEADOR);
    pthread_mutex_unlock(&BLOCKED);

	return (0);
}

int com_quit (char *arg){
	if(control_parametros(arg,0)){
		return 0;
	}
	done = 1;
	return (0);
}


void levantar_consola() {
	char *line,*s;
	done = 0;
	for (; done == 0;) {
		line = readline("Sentencia: ");
		if (!line)
			break;

		s = stripwhite(line);

		if (*s) {
			add_history(s);
			execute_line(s);
		}
		free(line);

	}
	puts("Adios consola");


}

int** inicializar_matriz(int cant_filas, int cant_columnas)
{
	int i = 0, j = 0;

	int ** matriz = malloc(cant_filas * sizeof(int));
	for(j = 0; j < cant_filas; j++)
	{
		matriz[j] = malloc(cant_columnas * sizeof(int));
	}
	for(i = 0; i < cant_filas; i++)
	{
		for(j = 0; j < cant_columnas; j++)
		{
			matriz[i][j] = 0;
		}
	}
	return matriz;
}

void mostrar_matriz(int** matriz,int filas, int columnas)
{
	int i, j;
	for(i = 0; i < filas; i++)
	{
		printf("ESI %d     ",i+1);
		for(j = 0; j < columnas; j++)
		{
			printf(" %d ",matriz[i][j]);
		}
		printf("\n");
	}
	printf("\n");
	printf("\n");
}

bool es_el_mismo_esi(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador){
	return nodo_bloqueado->esi->id == esi_bloqueador->esi->id;
}

bool tiene_lo_que_quiere(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador){
	return string_equals_ignore_case(nodo_bloqueado->clave, esi_bloqueador->clave);
}

bool generar_matriz_peticiones(t_list* bloqueadores, t_list* bloqueados, int ** matriz)
{

	t_esiBloqueador* esi_bloqueador_aux;
	t_nodoBloqueado* nodo_bloqueado_aux;

	int i,j,cant_claves,cant_esis;

	cant_claves = list_size(bloqueadores);

	cant_esis = list_size(bloqueados);

	i = 0;

	if(cant_esis > 1)
	{

		while(i < cant_esis)
		{
			nodo_bloqueado_aux = (t_nodoBloqueado*) list_get(bloqueados, i);
			j = 0;
			while(j < cant_claves)
			{
				esi_bloqueador_aux = (t_esiBloqueador*) list_get(bloqueadores, j);
				if(tiene_lo_que_quiere(nodo_bloqueado_aux, esi_bloqueador_aux))
				{
					matriz[i][j]=1;
					j=cant_claves;
				}

					j++;
			}
			i++;
		}

		return true;
	}
	else
	{
		return false;
	}

}

bool generar_matriz_asignados(t_list* bloqueadores, t_list* bloqueados, int** matriz) {

	t_esiBloqueador* esi_bloqueador_aux;
	t_nodoBloqueado* nodo_bloqueado_aux;

	int i = 0, j = 0, cant_claves, cant_esis;

	cant_claves = list_size(bloqueadores);

	cant_esis = list_size(bloqueados);

	if (cant_esis > 1) {

		while (i < cant_esis) {

			nodo_bloqueado_aux = (t_nodoBloqueado*) list_get(bloqueados, i);

			j = 0;
			while (j < cant_claves) {
				esi_bloqueador_aux = (t_esiBloqueador*) list_get(bloqueadores, j);
				if (es_el_mismo_esi(nodo_bloqueado_aux, esi_bloqueador_aux)) {
					matriz[i][j] = 1;
				}
				j++;
			}
			i++;
		}

		return true;
	} else {

		return false;
	}
}

void liberar_matriz(int** matriz,int c)
{
	int i=0;
	for(i=0;i<c;i++)
	{
		free(matriz[i]);
	}
	free(matriz);
}

t_list* get_esis_en_deadlock(t_list* bloqueadores, t_list* bloqueados)
{
	t_list* no_puede_ejecutar = list_create();

	if((list_size(bloqueados)) > 1)
	{
		int cant_esis, cant_claves, i, j;
		bool tiene_claves_tomadas = false;
		cant_esis = list_size(bloqueados);
		cant_claves = list_size(bloqueadores);
		int posible_deadlock[cant_esis];
		t_nodoBloqueado* nodo_bloqueado_aux;
		t_esiBloqueador* esi_bloqueador_aux;

		for(i = 0; i < cant_esis; i++){

			posible_deadlock[i] = 1;
		}

		for (i = 0; i < cant_esis; i++) {
			nodo_bloqueado_aux = (t_nodoBloqueado*) list_get(bloqueados, i);
			for (j = 0; j < cant_claves; j++) {
				esi_bloqueador_aux = (t_esiBloqueador*) list_get(bloqueadores, j);
				if (es_el_mismo_esi(nodo_bloqueado_aux, esi_bloqueador_aux)) {
					tiene_claves_tomadas = true;
					j = cant_claves;
				}
			}
			if (!tiene_claves_tomadas)
				posible_deadlock[i] = 0;
		}

		for (i = 0; i < cant_esis; i++) {
			if (posible_deadlock[i]) {
				nodo_bloqueado_aux = (t_nodoBloqueado*) list_get(bloqueados, i);
				for (j = 0; j < cant_claves; j++) {
					esi_bloqueador_aux = (t_esiBloqueador*) list_get(bloqueadores, j);
					if (tiene_lo_que_quiere(nodo_bloqueado_aux, esi_bloqueador_aux)) {
						list_add(no_puede_ejecutar, list_get(bloqueados, i));
						j = cant_claves;
					}
				}
			}
		}
	}

	return no_puede_ejecutar;
}

t_list* buscar_deadlock(t_list* bloqueadores,t_list* bloqueados){

	t_list* esis_en_deadlock = NULL;

    if(list_size(bloqueados) > 1)
    {

    	t_list* esis_aux = list_create();

    	t_list* claves_aux = list_create();

        list_add_all(esis_aux,bloqueados);

        list_add_all(claves_aux,bloqueadores);

    	int** matriz_peticiones = inicializar_matriz(list_size(bloqueados), list_size(bloqueadores));

		bool peticiones = generar_matriz_peticiones(claves_aux, esis_aux,matriz_peticiones);

    	int** matriz_recursos_asignados  = inicializar_matriz(list_size(bloqueados), list_size(bloqueadores));

    	bool asignados = generar_matriz_asignados(claves_aux, esis_aux,matriz_recursos_asignados);
/*
    	puts("Matriz de Peticiones");
    	puts("");

        mostrar_matriz(matriz_peticiones, list_size(bloqueados), list_size(bloqueadores));

        puts("Matriz de recursos asignados");
        puts("");

        mostrar_matriz(matriz_recursos_asignados, list_size(bloqueados), list_size(bloqueadores));
*/
    	if((asignados) && (peticiones))
    	{

        	esis_en_deadlock = get_esis_en_deadlock(claves_aux,esis_aux);

    		if(list_size(esis_en_deadlock) > 1)
			{

    			liberar_matriz(matriz_peticiones,list_size(bloqueados));

    			liberar_matriz(matriz_recursos_asignados,list_size(bloqueados));

    			list_destroy(claves_aux);

    			list_destroy(esis_aux);

    			printf("CANTIDAD DE ESIS EN DEADLOCK: %d\n", list_size(esis_en_deadlock));

    			return esis_en_deadlock;
			}

        	else

        	{

    			list_destroy(claves_aux);
    			list_destroy(esis_aux);
        		if(list_size(esis_en_deadlock) > 0){
        			list_clean(esis_en_deadlock);
        		}
        	}

    	}

    	else

    	{

			list_destroy(claves_aux);
			list_destroy(esis_aux);

    	}

    }

    return esis_en_deadlock;

}

