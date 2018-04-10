/*
 * cliente.c
 *
 *  Created on: 25/3/2018
 *      Author: utnso
 */

#include "cliente.h"

void envioMensaje(int* sockfd) {

    void*buffer = malloc(11);
    while(1){

        char* mensajeAEnviar = malloc(10 + 1);
        puts("Ingresa mensaje de 10 caracteres:\n");
        scanf("%s",mensajeAEnviar);
        mensajeAEnviar[strlen(mensajeAEnviar)] = '\0';

        memcpy(buffer,mensajeAEnviar,11);
        if(send((*sockfd), buffer,11, 0)== -1){
            puts("Error al enviar mensaje\n");
        }
        puts("Enviado OK\n");
    }
    

}

void levantarCliente(){

    int sockfd, numbytes;
    
    struct sockaddr_in their_addr; // información de la dirección de destino

    //1° Creamos un socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("No se pudo crear socket\n");
        exit(1);
    }
    puts("Sockete creado OK\n");
    their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
    their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
    //their_addr.sin_addr = *((struct in_addr *)he->h_addr);//toma la ip del atributo de la consola
    their_addr.sin_addr.s_addr = inet_addr(IP);//toma la ip directo

    memset( &(their_addr.sin_zero) , 0 , 8);  // poner a cero el resto de la estructura

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                                          sizeof(struct sockaddr)) == -1) {
        perror("No se pudo conectar\n");
        exit(1);
    }
    puts("Conectado OK\n");


    //CREAMOS UN HILO PARA ATENDERLO
    pthread_t punteroHilo;
    pthread_create(&punteroHilo, NULL, (void*) envioMensaje, &sockfd);

    char*buffer = malloc(11);
    while(1){
        if ((numbytes = recv(sockfd, buffer, 11, 0)) == -1) {
            perror("recv");
            printf("No se pudo recibir dato\n");
            exit(1);
        }
        printf("Recibi: %s\n", buffer);
	}


    free(buffer);
    close(sockfd);
}
