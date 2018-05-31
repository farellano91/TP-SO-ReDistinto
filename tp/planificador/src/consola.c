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

  char* clave = arg;

  return (0);
}

int com_desbloquear (char *arg){
	puts("Comando desbloquear!!");
	char* clave = arg;
	move_esi_from_bloqueado_to_listo(clave);
	//continua flujo si esta disponible el cpu
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
			pthread_mutex_unlock(&EXECUTE);
			continuar_comunicacion();
	}
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
		printf("La clave %s no se encuentra bloqueada\n", arg);
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
           cambio_de_lista(LIST_READY, LIST_FINISHED, esi_a_borrar->id);
           pthread_mutex_unlock(&READY);
           pthread_mutex_unlock(&BLOCKED);
		   free_recurso(esi_a_borrar->fd);
		   pthread_mutex_unlock(&EXECUTE);
		   printf("El ESI con id = %d fue eliminado\n", id_a_borrar);
		   return 0;
	}else if((esi_bloqueado_a_borrar = list_find(LIST_BLOCKED, (void*) _es_el_id_a_borrar_bloqueado)) != NULL){
		list_remove_by_condition(LIST_BLOCKED,(void*) _es_el_id_a_borrar_bloqueado);
		agregar_en_Lista(LIST_FINISHED, esi_bloqueado_a_borrar->esi);
		pthread_mutex_unlock(&READY);
		pthread_mutex_unlock(&BLOCKED);
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
	return (0);
}

int com_deadlock (char *arg){
	puts("Comando deadlock!!");
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


