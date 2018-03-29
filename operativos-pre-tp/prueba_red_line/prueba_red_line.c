/*
 ============================================================================
 Name        : prueba_red_line.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

void esTab ();

static char syscom[1024];

/* List the file(s) named in arg. */
com_list (arg)
     char *arg;
{
  if (!arg)
    arg = "";

  sprintf (syscom, "ls -FClg %s", arg);
  return (system (syscom));
}

int main() {
  char * linea = malloc(sizeof(char *));

  while(1) {
    linea = readline(">");

    //Si lo que recibo es list
	if (strcmp("ls", linea) == 0) {
		static char syscom[1024];
		sprintf (syscom, "ls");
		  (system (syscom));
	}
    //Si lo que recibo es exit
    if (strcmp("exit", linea) == 0) {
    	free(linea);
    	break;
    }

    //Evento que detecta el TAB (codigo ASCII) y ejecuta la funcion (No dejara el espacio del TAB al imprimir)
    rl_bind_key ('\t',  (void *)esTab );

    //Si quisiera respetar el espacio que deja el TAB sin disparar ningun evento
    //rl_bind_key ('\t',  rl_insert );

    printf("%s\n", linea);
    free(linea);
  }
  return 0;
};

void esTab(){
	printf("Presionaste un TAB gato\n");
}
