/*
 ============================================================================
 Name        : guia_ejercicio_2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //para el excev
#include <sys/types.h>
#include <sys/wait.h>

#include "archivo.h"

//se debe poder abrir (definir la forma: read/write/append), cerrar un archivo,
//leer una linea determinada, exponer una operación que reciba una función y la
//ejecute por cada línea del archivo; exponer otra operación que dada una lista
//y una función, itere la lista y escriba sobre el archivo abierto lo que devuelve
//dicha función (string).

int main(void) {
	int childStatus = 1;
	int ret = fork();
    if(ret == 0){
    	//pequeño cambiazo
    	char* args[] = { "/bin/mv", "prueba_archivo.md", "prueba_archivo.txt", 0 };
		execv(args[0], args);	
    }
    else{
    	//espero que el hijo termine para seguir
        waitpid(ret,&childStatus,0);
        FILE * archivo = txt_open_file("prueba_archivo.txt","r+b");
		//controlo que este bien
		if (!archivo){
			puts("Error al ver el archivo");
			return EXIT_SUCCESS;
		}
		txt_read_all(archivo);
		txt_close_file(archivo);

		//no paso nada	
		char* args[] = { "/bin/mv", "prueba_archivo.txt", "prueba_archivo.md", 0 };
		execv(args[0], args);
    }
	

//	//3.- Leo la linea 5
//	archivo = txt_open_file("prueba_archivo.txt","r");
//	//controlo que este bien
//	if (!archivo){
//		puts("Error al leer una linea espeficia del archivo");
//		return EXIT_SUCCESS;
//	}
//	txt_read_especific_line(archivo,4);
//	txt_close_file(archivo);


	return EXIT_SUCCESS;
}
