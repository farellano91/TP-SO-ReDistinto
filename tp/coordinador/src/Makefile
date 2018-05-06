#Declaramos variables
CC := gcc

OBJS := coordinador.o servidor.o funcionalidad_coordinador.o

#Generamos el principal
all: coordinador

coordinador: $(OBJS)
	gcc -o coordinador $(OBJS) -pthread -lcommons -lm

coordinador.o: coordinador.c servidor.h funcionalidad_coordinador.h
	gcc -c coordinador.c

servidor.o: servidor.c servidor.h funcionalidad_coordinador.h
	gcc -c servidor.c

funcionalidad_coordinador.o: funcionalidad_coordinador.c funcionalidad_coordinador.h
	gcc -c funcionalidad_coordinador.c


#Borramos todo
clear: 
	rm coordinador $(OBJS)
