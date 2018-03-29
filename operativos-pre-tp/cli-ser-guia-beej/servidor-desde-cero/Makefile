#Declaramos variables
CC := gcc

OBJS := servidor.o

#Generamos el principal
all: ejecutableServidor

ejecutableServidor: $(OBJS)
	gcc -o ejecutableServidor $(OBJS) -pthread

servidor.o: servidor.c
	gcc -c servidor.c

#Borramos todo
clear: 
	rm ejecutableServidor servidor.o
