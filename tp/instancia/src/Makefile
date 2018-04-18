#Declaramos variables
CC := gcc

OBJS := instancia.o cliente.o funcionalidad_instancia.o

#Generamos el principal
all: instancia

instancia: $(OBJS)
	$(CC) -o instancia $(OBJS) -lcommons

instancia.o: instancia.c cliente.h funcionalidad_instancia.h
	$(CC) -c instancia.c

cliente.o: cliente.c cliente.h funcionalidad_instancia.h
	$(CC) -c cliente.c

funcionalidad_instancia.o: funcionalidad_instancia.c funcionalidad_instancia.h
	$(CC) -c funcionalidad_instancia.c
#Borramos todo
clear: 
	rm instancia $(OBJS)
