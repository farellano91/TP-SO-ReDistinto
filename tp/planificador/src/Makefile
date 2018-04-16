#Declaramos variables
CC := gcc

OBJS := planificador.o cliente.o consola.o

#Generamos el principal
all: planificador

planificador: $(OBJS)
	$(CC) -o planificador $(OBJS) -lcommons -lreadline -lpthread

planificador.o: planificador.c cliente.h consola.h
	$(CC) -c planificador.c

cliente.o: cliente.c cliente.h
	$(CC) -c cliente.c

consola.o: consola.c consola.h
	$(CC) -c consola.c

#Borramos todo
clear: 
	rm planificador $(OBJS)
