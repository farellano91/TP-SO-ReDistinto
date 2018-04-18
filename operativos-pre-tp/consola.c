#include "consola.h"

typedef int Function();
typedef char **CPPFunction();

/* The names of functions that actually do the manipulation. */
int com_pausa(), com_continuar(), com_bloquear(), com_desbloquear(),
		com_listar();
int com_kill(), com_status(), com_deadlock(), com_quit();

/* A structure which contains information on the commands this program
 can understand. */

typedef struct {
	char *name; /* User printable name of the function. */
//  Function *func;     /* Function to call to do the job. */
	int (*func)();
	int params; /* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
		{ "pausa", com_pausa, 1 },
		{ "continuar", com_continuar, 1 },
		{ "bloquear", com_bloquear, 1 },
		{ "desbloquear", com_desbloquear, 1 },
		{ "listar", com_listar, 1 },
		{ "kill", com_kill, 1 },
		{ "status", com_status, 1 },
		{ "deadlock", com_deadlock, 1 },
		{ "quit", com_quit, 1 },
		{ (char *) NULL, NULL, (int)0} };

/* Forward declarations. */
char *stripwhite();
COMMAND *find_command();

/* The name of this program, as taken from argv[0]. */
char *progname;

/* When non-zero, this global means the user is done using this program. */
int done;

/* Execute a command line. */
int execute_line(char *line) {
	register int i;
	COMMAND *command;
	char *word;

	/* Isolate the command word. */
	i = 0;
	while (line[i] && whitespace(line[i]))
		i++;
	word = line + i;

	while (line[i] && !whitespace(line[i]))
		i++;

	if (line[i])
		line[i++] = '\0';

	command = find_command(word);

	if (!command) {
		fprintf(stderr, "Comando no reconocido!.\n");
		return (-1);
	}

	/* Get argument to command, if any. */
	while (whitespace(line[i]))
		i++;

	word = line + i;

	/* Call the function. */
	return ((*(command->func))(word));
}

/* Look up NAME as the name of a command, and return a pointer to that
 command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND * find_command(char *name) {
	register int i;

	for (i = 0; commands[i].name; i++)
		if (strcmp(name, commands[i].name) == 0)
			return (&commands[i]);

	return ((COMMAND *) NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
 into STRING. */
char * stripwhite(char *string) {
	register char *s, *t;

	for (s = string; whitespace(*s); s++)
		;

	if (*s == 0)
		return (s);

	t = s + strlen(s) - 1;
	while (t > s && whitespace(*t))
		t--;
	*++t = '\0';

	return s;
}

int com_pausa(char *arg) {
	puts("Comando pausa!!");
	return (0);
}

int com_continuar(char *arg) {
	puts("Comando continuar!!");
	return (0);
}

int com_bloquear(char *arg) {
	puts("Comando bloquear ingresado!!");
	return (0);
}

int com_desbloquear(char *arg) {
	puts("Comando desbloquear!!");
	return (0);
}

int com_listar(char *arg) {
	puts("Comando listar!!");
	return (0);
}

void validar_parametros(char* parametros,char* name) {
	int i;
	for (i = 0; commands[i].name; i++){
		if(strcmp(name,commands[i].name) == 0 ){
			int cant = 0;
			int posicion = 0;
			while(parametros[posicion] != '\0'){
				if(parametros[posicion] == ' '){
					cant++;
				}
				posicion++;
			}
			//sin parametros
			if(parametros[0] != '\0' ){
				cant++;
			}
			if(cant != (commands[i].params)){
				puts("Parametros no correctos");
			}
			break;
		}
	}



}
//kill <paramtro1>
int com_kill(char *arg) {
	validar_parametros(arg,"kill");
	puts("Comando kill!!");
	return (0);
}

int com_status(char *arg) {
	puts("Comando status!!");
	return (0);
}

int com_deadlock(char *arg) {
	puts("Comando deadlock!!");
	return (0);
}

int com_quit(char *arg) {
	done = 1;
	return (0);
}

void levantar_consola() {
	char *line, *s;
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

}

