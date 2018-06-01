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
	pthread_mutex_lock(&READY);
	pthread_mutex_lock(&EXECUTE);
	move_esi_from_bloqueado_to_listo(clave);
	//continua flujo si esta disponible el cpu
	if (list_is_empty(LIST_EXECUTE) && !list_is_empty(LIST_READY)) {
			list_add(LIST_EXECUTE,list_get(LIST_READY, 0));
			list_remove(LIST_READY, 0);
			pthread_mutex_unlock(&EXECUTE);
			pthread_mutex_unlock(&READY);
			continuar_comunicacion();
	}
	pthread_mutex_unlock(&EXECUTE);
	pthread_mutex_unlock(&READY);
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
		agregar_en_Lista(LIST_FINISHED, esi_bloqueado_a_borrar->esi);

		pthread_mutex_unlock(&BLOCKED);
		pthread_mutex_unlock(&READY);

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
	pthread_mutex_unlock(&BLOCKED);
	pthread_mutex_unlock(&READY);

	return (0);
}

int com_status (char *arg){
	puts("Comando status!!");
	return (0);
}

int com_deadlock (char *arg){

	void _imprimir_id(t_nodoBloqueado* nodo){
		printf("ESI id = %d\n", nodo->esi->id);
	}

	pthread_mutex_lock(&BLOCKED);

    t_list* lista = buscar_deadlock(LIST_ESI_BLOQUEADOR, LIST_BLOCKED);

    if(lista != NULL){
       puts("Deadlock detectado. Los ESIs implicados son:");
       list_iterate(lista, (void*)_imprimir_id);
    }else{
    	puts("No se ha detectado un deadlock");
    }

    pthread_mutex_unlock(&BLOCKED);

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


bool es_el_mismo_esi(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador){
	return nodo_bloqueado->esi->id == esi_bloqueador->esi->id;
}

bool tiene_lo_que_quiere(t_nodoBloqueado* nodo_bloqueado,t_esiBloqueador* esi_bloqueador){
	return string_equals_ignore_case(nodo_bloqueado->clave, esi_bloqueador->clave);
}

t_list* get_esis_en_deadlock(t_list* bloqueadores, t_list* bloqueados, int** matriz_peticiones, int** matriz_recursos_asignados/*, int* recursos_disponibles*/)
{
	t_list* no_puede_ejecutar = list_create();

	if((list_size(bloqueados)) > 1)
	{
		int cant_esis, cant_claves;
		bool tiene_claves_tomadas = false;
		cant_esis = list_size(bloqueados);
		cant_claves = list_size(bloqueadores);
		int posible_deadlock[cant_esis];
		int i = 0, j, k;
		int tamanio = list_size(bloqueadores);

		for(i = 0; i < cant_esis; i++){

			posible_deadlock[i] = 1;
		}


		for(i = 0; i < cant_esis; i++){

			tiene_claves_tomadas = false;
			for(j = 0; j < cant_claves; j++){

				if(matriz_recursos_asignados[i][j])
					tiene_claves_tomadas = true;
			}

			if(!tiene_claves_tomadas)
				posible_deadlock[i] = 0;
		}

		for(i = 0; i < cant_esis; i++){

			if(posible_deadlock[i])
			{
				list_add(no_puede_ejecutar,list_get(bloqueados,i));
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

    	t_list* claves_aux=list_create();

        list_add_all(esis_aux,bloqueados);

        list_add_all(claves_aux,bloqueadores);

    	int** matriz_peticiones = inicializar_matriz(list_size(bloqueados), list_size(bloqueadores));

		bool peticiones = generar_matriz_peticiones(claves_aux, esis_aux,matriz_peticiones);

    	int** matriz_recursos_asignados  = inicializar_matriz(list_size(bloqueados), list_size(bloqueadores));

    	bool asignados = generar_matriz_asignados(claves_aux, esis_aux,matriz_recursos_asignados);

    	puts("Matriz de Peticiones");
    	puts("");

        mostrar_matriz(matriz_peticiones, list_size(bloqueados), list_size(bloqueadores));

        puts("Matriz de recursos asignados");
        puts("");

        mostrar_matriz(matriz_recursos_asignados, list_size(bloqueados), list_size(bloqueadores));

    	if((asignados) && (peticiones))
    	{

        	esis_en_deadlock = get_esis_en_deadlock(claves_aux,esis_aux,matriz_peticiones,matriz_recursos_asignados/*,recursos_disponibles*/);

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

