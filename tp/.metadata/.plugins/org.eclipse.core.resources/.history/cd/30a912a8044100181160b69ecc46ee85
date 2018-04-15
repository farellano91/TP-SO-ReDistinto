#Declaramos variables
CC := gcc

OBJS := planificador.o cliente.o

#Generamos el principal
all: planificador

planificador: $(OBJS)
	$(CC) -o planificador $(OBJS) -lcommons

planificador.o: planificador.c cliente.h
	$(CC) -c planificador.c

cliente.o: cliente.c cliente.h
	$(CC) -c cliente.c
#Borramos todo
clear: 
	rm planificador $(OBJS)
