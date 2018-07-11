/*
 * archivo.c
 *
 *  Created on: 31/3/2018
 *      Author: utnso
 */


#include "archivo.h"


FILE* txt_open_file(char* path, char* condicion) {
	return fopen(path,condicion);
}


void txt_close_file(FILE* file) {
	fclose(file);
}

void txt_read_especific_line(FILE* file, int line_number){
	char* line = malloc(sizeof(char)*256);
	int contador = 1;
	while (fgets(line, 256, file)) {

		if(contador == line_number){
			printf("Fila n° %d elegida: %s\n",line_number,line);
			break;
		}
		contador++;
	}
	free(line);


}





