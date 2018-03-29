
    /*
    ** client.c -- Ejemplo de cliente de sockets de flujo
    */

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>

    #define PORT 8082 // puerto al que vamos a conectar

    #define MAXDATASIZE 100 // máximo número de bytes que se pueden leer de una vez

    int main(int argc, char *argv[])
    {
        int sockfd, numbytes;
        char* buf = malloc(MAXDATASIZE);
        struct hostent *he;
        struct sockaddr_in their_addr; // información de la dirección de destino

//        if (argc != 2) {
//            fprintf(stderr,"usage: client hostname\n");
//            exit(1);
//        }
//
//        if ((he=gethostbyname(argv[1])) == NULL) {  // obtener información de máquina
//            perror("gethostbyname");
//            exit(1);
//        }

        //1° Creamos un socket
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        their_addr.sin_family = AF_INET;    // Ordenación de bytes de la máquina
        their_addr.sin_port = htons(PORT);  // short, Ordenación de bytes de la red
        //their_addr.sin_addr = *((struct in_addr *)he->h_addr);//toma la ip del atributo de la consola
        their_addr.sin_addr.s_addr = inet_addr("192.168.0.12");//toma la ip directo

        memset( &(their_addr.sin_zero) , 0 , 8);  // poner a cero el resto de la estructura

        if (connect(sockfd, (struct sockaddr *)&their_addr,
                                              sizeof(struct sockaddr)) == -1) {
            perror("connect");
            printf("No se pudo conectar\n");
            exit(1);
        }
        //Recibe y lo guarda en buffer
        if ((numbytes=recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            printf("No se pudo recibir dato\n");
            exit(1);
        }
        printf("Recibi: %s",buf);

        while(1){
        	char mensajeParaEnviar[1000];
			scanf("%s",mensajeParaEnviar);
			if (strcmp(mensajeParaEnviar, "exit") == 0){
				send(sockfd, mensajeParaEnviar,strlen(mensajeParaEnviar), 0);
				break;
			}
			send(sockfd, mensajeParaEnviar,strlen(mensajeParaEnviar), 0);

        }

        close(sockfd);

        return 0;
    }
