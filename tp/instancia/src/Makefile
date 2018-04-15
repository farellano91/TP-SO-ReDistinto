#Declaramos variables
CC := gcc

OBJS := instancia.o cliente.o

#Generamos el principal
all: instancia

instancia: $(OBJS)
	$(CC) -o instancia $(OBJS) -lcommons

instancia.o: instancia.c cliente.h
	$(CC) -c instancia.c

cliente.o: cliente.c cliente.h
	$(CC) -c cliente.c
#Borramos todo
clear: 
	rm instancia $(OBJS)
