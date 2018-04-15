/*
 * servidor.c no multihilo
 *
 *  Created on: 25/3/2018
 *      Author: utnso
 */
#include "servidor.h"

void sigchld_handler(int s) {
	while (wait(NULL) > 0)
		;
}

/*
FD_ZERO(fd_set *set) -- borra un conjunto de descriptores de fichero

FD_SET(int fd, fd_set *set) -- añade fd al conjunto

FD_CLR(int fd, fd_set *set) -- quita fd del conjunto

FD_ISSET(int fd, fd_set *set) -- pregunta si fd está en el conjunto
*/


/*Este servidor recibe un mensaje y lo manda en eco a todos los demas clientes que tenga conectados*/
void levantarServidor() {
	int sockfd; // Escuchar sobre: sock_fd, nuevas conexiones sobre: idSocketCliente
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del idSocketCliente
	int sin_size;
	struct sigaction sa;
	int yes = 1;
	int fdmax;
	fd_set master;   // conjunto maestro de descriptores de fichero
    fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
    FD_ZERO(&master);    // borra los conjuntos maestro y temporal
    FD_ZERO(&read_fds);
    int j;
    int i;
	//1° CREAMOS EL SOCKET
	//sockfd: numero o descriptor que identifica al socket que creo
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		printf("Error al abrir el socket de escucha\n");
		exit(1);
	}
	printf("Se creo el socket %d\n", sockfd);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("address already in use");
		exit(1);
	}

	my_addr.sin_family = PF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(MYPORT);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(MYIP); //INADDR_ANY (aleatoria) o 127.0.0.1 (local)
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	//2° Relacionamos los datos de my_addr <=> socket
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
		printf("Fallo el bind\n");
		perror("bind");
		exit(1);
	}

	//3° Listen: se usa para dejar al socket escuchando las conexiones que se acumulan en una cola hasta que
	//la aceptamos
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		printf("Fallo el listen\n");
		exit(1);
	}
	printf("Socket escuchando!!!\n");

	//-------
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	//--------
	sin_size = sizeof(struct sockaddr_in);

	// añadir el socket creado al conjunto maestro
    FD_SET(sockfd, &master);
    // seguir la pista del descriptor de fichero mayor
    fdmax = sockfd; // por ahora es éste
    // bucle principal
    for(;;) {
        read_fds = master; // copi el conjunto maestro como temporal
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) { //se encarga de llenar en read_fds todos los fd cliente que cambiaron
            perror("select");
            exit(1);
        }
        // explorar conexiones existentes en busca de datos que leer
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // recorro toda la bolsa read_fds
                if (i == sockfd) { // el que cambio es el fd del socket q cree y deje en listen, por ende, escucho algo ->
                    // NUEVA CONEXION!!
                    //addrlen = sizeof(their_addr);
                    int socketCliente;
                    if ((socketCliente = accept(sockfd, (struct sockaddr *)&their_addr,
                                                             &sin_size)) == -1) { 
                        perror("ERROR: Al ejecutar -> accept");
                    } else {
                        FD_SET(socketCliente, &master); // añadir al conjunto maestro ya q desde ahora en adelante lo vamos a usar para recibir
                        if (socketCliente > fdmax) {    // actualizar el máximo deacuerdo al fd del cliente 
                            fdmax = socketCliente;
                        }
                        printf("Se conecto el cliente de ip: %s en "
                            "socket numero: %d\n", inet_ntoa(their_addr.sin_addr), socketCliente);
                    }
                } else { //si el fd que cambio es diferente del que esta en listen, entonces
                	//significa que un cliente esta mandando algo
                	int nbytes;
                	void* buf = malloc(11);
                    if ((nbytes = recv(i, buf,11, 0)) <= 0) {
                        // recibo CERO -> error o conexión cerrada por el cliente
                        if (nbytes == 0) {
                            // conexión cerrada
                            printf("Se fue: socket %d, chau gato!!!\n", i);
                        } else {
                            perror("ERROR: al recibir info del cliente");
                        }
                        close(i); // si ya no conversare mas con el cliente, lo cierro
                        FD_CLR(i, &master); // eliminar del conjunto maestro
                    } else {
                    	printf("Recibi correctamente el mensaje\n");
                        // tenemos en buf los datos recibidos del cliente i
                        for(j = 0; j <= fdmax; j++) {
                            // recorro mi bolda de fd fijos para hacer reenvio a todos
                            if (FD_ISSET(j, &master)) {
                                // obviamente el reenvio es solo para los clientes que no sea yo mismo (sockfd) ni el q me mando
                                // el mensaje (i), a menos q asi lo querramos
                                if (j != sockfd && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("ERROR: al enviar mensaje recibido a todos los demas clientes\n");
                                    }
                                    printf("Mensajes reenviado a: %d\n", j);

                                }
                                
                            }
                        }
                    }
                  	free(buf);
                } 
            }
        }
    }
    close(sockfd);
 }