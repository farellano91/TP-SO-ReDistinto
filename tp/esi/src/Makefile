#Declaramos variables
CC := gcc

OBJS := esi.o archivo.o cliente.o funcionalidad_esi.o

#Generamos el principal
all: esi

esi: $(OBJS)
	$(CC) -o esi $(OBJS) -lcommons -lparsi

esi.o: esi.c archivo.h cliente.h funcionalidad_esi.h
	$(CC) -c esi.c

archivo.o: archivo.c archivo.h
	$(CC) -c archivo.c

cliente.o: cliente.c cliente.h funcionalidad_esi.h
	$(CC) -c cliente.c

funcionalidad_esi.o: funcionalidad_esi.c funcionalidad_esi.h
	$(CC) -c funcionalidad_esi.c
#Borramos todo
clear: 
	rm esi $(OBJS)
