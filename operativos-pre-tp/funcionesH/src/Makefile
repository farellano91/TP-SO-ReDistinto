#Declaramos variables
CC := gcc

OBJS := funciones.o mainFuncionesH.o

#Generamos el principal
all: ejecutableMain

ejecutableMain: $(OBJS)
	$(CC) -o ejecutableMain $(OBJS)

mainFuncionesH.o: mainFuncionesH.c funciones.h
	$(CC) -c mainFuncionesH.c

funciones.o: funciones.c funciones.h
	$(CC) -c funciones.c

#Borramos todo
clearTodo:
	rm ejecutableMain funciones.o mainFuncionesH.o


#-----------------------------------------------------------
#En el principal (ejecutableMain) van como dependecias todos los .o
#de los .c de si mismo y de los que esten dentro
#de los .h q use Y de ejecuta -o principal y todos los .o

#En los .o que 	quiero crear van como dependencia a si mismo .c y los .h
#que use Y ejecuta -c de si mismmo.c


#se ejecuta la primera regla con make y las otras com
#make clearTodo
