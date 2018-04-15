#Declaramos variables
CC := gcc

OBJS := esi.o archivo.o cliente.o

#Generamos el principal
all: esi

esi: $(OBJS)
	$(CC) -o esi $(OBJS) -lcommons

esi.o: esi.c archivo.h cliente.h
	$(CC) -c esi.c

archivo.o: archivo.c archivo.h
	$(CC) -c archivo.c

cliente.o: cliente.c cliente.h
	$(CC) -c cliente.c
#Borramos todo
clear: 
	rm esi $(OBJS)
