#Declaramos variables
CC := gcc

OBJS := planificador.o cliente.o consola.o servidor.o funcionalidad_planificador.o

#Generamos el principal
all: planificador

planificador: $(OBJS)
	$(CC) -o planificador $(OBJS) -lcommons -lreadline -lpthread

planificador.o: planificador.c cliente.h consola.h servidor.h funcionalidad_planificador.h
	$(CC) -c planificador.c

cliente.o: cliente.c cliente.h funcionalidad_planificador.h
	$(CC) -c cliente.c

servidor.o: servidor.c servidor.h funcionalidad_planificador.h
	$(CC) -c servidor.c

funcionalidad_planificador.o: funcionalidad_planificador.c funcionalidad_planificador.h
	$(CC) -c funcionalidad_planificador.c
	
consola.o: consola.c consola.h
	$(CC) -c consola.c

#Borramos todo
clear: 
	rm planificador $(OBJS)
