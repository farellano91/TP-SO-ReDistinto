#Declaramos variables
CC := gcc

OBJS := moduloServidor.o servidor.o

#Generamos el principal
all: ejecutableServidor

ejecutableServidor: $(OBJS)
	gcc -o ejecutableServidor $(OBJS) -pthread

servidor.o: servidor.c servidor.h
	gcc -c servidor.c

moduloServidor.o: moduloServidor.c servidor.h
	gcc -c moduloServidor.c

#Borramos todo
clear: 
	rm ejecutableServidor servidor.o moduloServidor.o
