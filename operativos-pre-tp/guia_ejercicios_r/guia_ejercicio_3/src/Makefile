#Declaramos variables
CC := gcc

OBJS := guia_ejercicio_3.o archivo.o

#Generamos el principal
all: ejecutable

ejecutable: $(OBJS)
	$(CC) -o ejecutable $(OBJS) -lcommons

guia_ejercicio_3.o: guia_ejercicio_3.c archivo.h
	$(CC) -c guia_ejercicio_3.c

archivo.o: archivo.c archivo.h
	$(CC) -c archivo.c -lcommons

#Borramos todo
clearTodo:
	rm ejecutable archivo.o guia_ejercicio_3.o


#-----------------------------------------------------------
#En el principal (ejecutableMain) van como dependecias todos los .o
#de los .c de si mismo y de los que esten dentro
#de los .h q use Y de ejecuta -o principal y todos los .o

#En los .o que 	quiero crear van como dependencia a si mismo .c y los .h
#que use Y ejecuta -c de si mismmo.c


#se ejecuta la primera regla con make y las otras com
#make clearTodo
