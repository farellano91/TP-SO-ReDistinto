#Declaramos variables
CC := gcc

OBJS := moduloCliente.o cliente.o

#Generamos el principal
all: ejecutableCliente

ejecutableCliente: $(OBJS)
	$(CC) -o ejecutableCliente $(OBJS) 

cliente.o: cliente.c cliente.h
	$(CC) -c cliente.c

moduloCliente.o: moduloCliente.c cliente.h
	$(CC) -c moduloCliente.c

#Borramos todo
clear: 
	rm ejecutableCliente cliente.o moduloCliente.o
