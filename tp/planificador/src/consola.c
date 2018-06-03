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
	puts("Comando pausa!!");
	pthread_mutex_lock(&MUTEX);
	PLANIFICADOR_EN_PAUSA = true;
	pthread_mutex_unlock(&MUTEX);
	return (0);
}

int com_continuar (char *arg){
	puts("Comando continuar!!");
	pthread_mutex_lock(&MUTEX);
	PLANIFICADOR_EN_PAUSA = false;
	pthread_mutex_unlock(&MUTEX);
	pthread_cond_signal(&CONDICION_PAUSA_PLANIFICADOR);
	return (0);
}

int com_bloquear (char *arg){
  puts("Comando bloquear ingresado!!");
  return (0);
}

int com_desbloquear (char *arg){
	puts("Comando desbloquear!!");
	char* clave = arg;

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
	return (0);
}

int com_listar (char *arg){

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
		pthread_mutex_unlock(&READY);
		pthread_mutex_unlock(&BLOCKED);

		printf("El ESI con id = %d fue eliminado\n", id_a_borrar);

		return 0;

	}else if((esi_a_borrar = list_find(LIST_READY, (void*) _es_el_id_a_borrar)) != NULL) {

		   pthread_mutex_unlock(&READY);
           pthread_mutex_unlock(&BLOCKED);
           pthread_mutex_unlock(&EXECUTE);

           move_esi_from_ready_to_finished(esi_a_borrar->id);
		   free_recurso(esi_a_borrar->fd);

		   pthread_mutex_lock(&SOCKETS);
           list_add(LIST_SOCKETS,(void*)esi_a_borrar->fd);
		   pthread_mutex_unlock(&SOCKETS);

		   printf("El ESI con id = %d fue eliminado\n", id_a_borrar);

		   return 0;

	}else if((esi_bloqueado_a_borrar = list_find(LIST_BLOCKED, (void*) _es_el_id_a_borrar_bloqueado)) != NULL){

		list_remove_by_condition(LIST_BLOCKED,(void*) _es_el_id_a_borrar_bloqueado);
		agregar_en_Lista(LIST_FINISHED, esi_bloqueado_a_borrar->esi);

		pthread_mutex_unlock(&READY);
		pthread_mutex_unlock(&BLOCKED);

		int fd = esi_bloqueado_a_borrar->esi->fd;
		free_recurso(esi_bloqueado_a_borrar->esi->fd);
		free_nodoBLoqueado(esi_bloqueado_a_borrar);

		pthread_mutex_lock(&READY);

		if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {

			    list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
				list_remove(LIST_READY, 0);
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
	pthread_mutex_unlock(&READY);
	pthread_mutex_unlock(&BLOCKED);

	return (0);
}

int com_status (char *arg){
	puts("Comando status!!");
	char* clave = arg;
	int32_t longitud_clave = strlen(clave) + 1;
	void* bufferEnvio = malloc(sizeof(int32_t) + longitud_clave);
	memcpy(bufferEnvio, &longitud_clave,sizeof(int32_t));
	memcpy(bufferEnvio + sizeof(int32_t),clave,longitud_clave);

	if( FD_COORDINADOR_STATUS == -1 ){
		printf("No tengo el FD del coordinador status\n");
		return (0);
	}
	if (send(FD_COORDINADOR_STATUS, bufferEnvio,sizeof(int32_t) + longitud_clave, 0) == -1) {
		printf("No se pudo enviar pedido status\n");
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
				printf("Valor: No existe la clave en el sistema\n");
				printf("Instancia: No existe la clave en el sistema\n");

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
				printf("Valor: - \n");
				printf("Instancia elegida por algoritmo: %s\n",nombre_instancia);
				free(nombre_instancia);
				break;
			case 5://5: la instancia es detectada con el algoritmo es null, osea no hay, y ademas (como es nueva no tiene valor)
				printf("Valor: -\n");
				printf("Instancia elegida por algoritmo: - \n");

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

	void _buscarDeadlock(t_esiBloqueador* esiBloqueador){

		void _estanEnDeadlock(t_nodoBloqueado* nodo){
	        if(string_equals_ignore_case(nodo->clave, esiBloqueador->clave) && quiereAlgoQueElOtroTiene(esiBloqueador, nodo)){
               printf("El ESI %d está en deadlock con el ESI %d\n", esiBloqueador->esi->id, nodo->esi->id);
	        }
		}

		pthread_mutex_lock(&BLOCKED);

        list_iterate(LIST_BLOCKED, (void*)_estanEnDeadlock);

		pthread_mutex_unlock(&BLOCKED);

	}

	list_iterate(LIST_ESI_BLOQUEADOR, (void*)_buscarDeadlock);

	return (0);
}

int com_quit (char *arg){
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


