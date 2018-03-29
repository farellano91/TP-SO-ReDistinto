#Declaramos variables
CC := gcc

OBJS := prueba_red_line.o

#Generamos el principal
all: ejecutar

ejecutar: $(OBJS)
	gcc -o ejecutable $(OBJS) -lreadline

prueba_red_line.o: prueba_red_line.c
	gcc -c prueba_red_line.c

#Borramos todo
clear:
	rm ejecutable prueba_red_line.o
