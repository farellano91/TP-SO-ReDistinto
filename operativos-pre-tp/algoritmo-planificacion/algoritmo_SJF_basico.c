#include<stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc
#include <unistd.h>


int main(int n, char **args) {
	printf("SJF: SHORTEST JOB FIRST");
	// cada proceso contiene dos elementos:
	//  - Posición 0 el tiempo del trabajo
	// 	- Posición 1 (la posición original de llegada.
	int np=11, procesos[10][2];
	double tf = 0, tp;// tiempo promedio.
	while (np > 10 || np <= 0) {
		printf("\nNumero de procesos: ");
		scanf("%d", &np);
	}
	//para i=0, mientras i<np, hacer:...
	// pedimos el tamaño de cpu de cada proceso.
	for(int i=0; i<np; i++) {
		printf("\nInserte cpu para el proceso %d :", i+1);
		scanf("%d", &procesos[i][0]);
		procesos[i][1] = i+1;
	}
	// Algoritmo SJF
	// ordenamos de menor a mayor
	for (int i=0; i<np-1; i++) {
		for(int j=i+1; j<np; j++) {
			if (procesos[j][0]<procesos[i][0]) {
				int aux[2] = {procesos[j][0], procesos[j][1]};
				procesos[j][0] = procesos[i][0];
				procesos[j][1] = procesos[i][1];
				procesos[i][0] = aux[0];
				procesos[i][1] = aux[1];
			}
		}
	}
	for (int i=0; i<np; i++) {
		printf("\nProceso %d trabajando", procesos[i][1]);
		printf("\nTrabajando...");
		fflush(stdout);
		sleep(procesos[i][0]);
		printf("\nProceso %d hizo:%d y finalizo!", procesos[i][1],procesos[i][0]);
		printf("\n-------------------------------");
	}
	
	return 0;
}
