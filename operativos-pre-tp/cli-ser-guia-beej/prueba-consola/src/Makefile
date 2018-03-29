#Declaramos variables
CC := gcc

OBJS := prueba-consola.o

#Generamos el principal
all: ejecutable

ejecutable: $(OBJS)
	$(CC) -o ejecutable $(OBJS) -lreadline

prueba-consola.o: prueba-consola.c
	$(CC) -c prueba-consola.c

#Borramos todo
clear:
	rm ejecutable prueba-consola.o
