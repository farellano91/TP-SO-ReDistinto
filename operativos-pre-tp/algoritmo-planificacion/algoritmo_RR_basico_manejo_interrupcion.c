
#include <stdio.h> // Por dependencia de readline en algunas distros de linux :)
#include <string.h>
#include <stdlib.h> // Para malloc
#include <signal.h>

#include <unistd.h>

typedef struct {
  int id;
  int cpu;
} __attribute__((packed)) Proceso;


void intHandler(int dummy) {
    if(dummy != 0){
    	printf("\nFinalizo con una interrupcion :(, codigo: %d!!\n",dummy);
    	exit(dummy);
    }
}

//Modelo RR para 10 procesos
int main(int n, char **args) {

	//En caso de una interrupcion va por aca
	signal(SIGINT, intHandler);

	printf("Round Robin");
	int np=11, quantum = 0, nq = 0;
	Proceso  proceso[10];

	double tp = 0;// tiempo promedio.
	int finalizado = 0; //bandera para saber si alguno de la lista aun tiene cpu 
	while (np > 10 || np <= 0) {
		printf("\nNumero de proceso(%d): ", np);
		scanf("%d", &np);
	}
	//para i=0, mientras i<np, hacer:...
	// pedimos el tamaño de cada proceso.
	for(int i=0; i<np; i++) {
		printf("\nIngrese ID del proceso %d:", i+1);
		scanf("%d", &(proceso[i].id));

		printf("\nInserte cant. de CPU para el proceso %d:", i+1);
		scanf("%d",  &(proceso[i].cpu));
	}
	while (quantum <= 0) {
		printf("\nTamaño de quantum:");
		scanf("%d", &quantum);
	}
	// Algoritmo RR
	int cpuRestante = 0;

	while(finalizado == 0) {
		finalizado = 1;
		for(int i=0; i<np; i++) {
				
				if(proceso[i].cpu > 0){

					cpuRestante = proceso[i].cpu - quantum;
					if (cpuRestante>0) {
						sleep(quantum);
						printf("\nProceso ID:%d realizó:%d",proceso[i].id,quantum);
						proceso[i].cpu = cpuRestante;
						tp = tp + quantum;
						finalizado = 0; //aun le queda cpu
					}

					
					else{
						sleep(proceso[i].cpu);
						printf("\nProceso ID:%d realizó:%d ---> Finalizó",proceso[i].id,quantum);
						tp = tp + proceso[i].cpu;
						proceso[i].cpu = cpuRestante;
					}
					
				}
				
		}
	}
	printf("\nTiempo total RR :%f\n", tp);
	return 0;
}