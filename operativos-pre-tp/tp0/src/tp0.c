#include "tp0.h"

int main() {
  configure_logger();
  int socket = connect_to_server(IP, PUERTO);
  wait_hello(socket);
  Alumno alumno = read_hello();
  send_hello(socket, alumno);
  void * content = wait_content(socket);
  send_md5(socket, content);
  wait_confirmation(socket); 
  exit_gracefully(0);
  return 0;
}

void configure_logger() {
  logger = log_create("log_tp0.log","tp0",1,LOG_LEVEL_INFO);
  log_info(logger, "Creado xD\n");

}

int connect_to_server(char * ip, char * port) {
  struct addrinfo hints;
  struct addrinfo *server_info;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;    // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
  hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

  getaddrinfo(ip, port, &hints, &server_info);  // Carga en server_info los datos de la conexion

  // 2. Creemos el socket con el nombre "server_socket" usando la "server_info" que creamos anteriormente
  
  /*TODO: este es el socket que a futuro se espera cerrar???*/
  int server_socket = socket(server_info->ai_family, server_info->ai_socktype, 0);
  if (server_socket == -1) {
	  	log_error(logger, "Error: al crear el socket server_socket");
        freeaddrinfo(server_info); 
        exit_gracefully(1);
  }
  // 3. Conectemosnos al server a traves del socket! Para eso vamos a usar connect()
  int retorno = connect(server_socket,server_info->ai_addr, server_info->ai_addrlen);
  if (retorno  == -1) {
	    log_error(logger, "Error: al llamar a connect");
        freeaddrinfo(server_info); 
        exit_gracefully(1);
    }

  freeaddrinfo(server_info);  // No lo necesitamos mas, lo libera

  /*
    3.1 Recuerden chequear por si no se pudo contectar (usando el retorno de connect()).
        Si hubo un error, lo loggeamos y podemos terminar el programa con la funcioncita
        exit_gracefully pasandole 1 como parametro para indicar error ;).
        Pss, revisen los niveles de log de las commons.
  */

  // 4 Logeamos que pudimos conectar y retornamos el socket
  log_info(logger, "Conectado!");
  return server_socket;
}

void  wait_hello(int socket) {
  char * hola = malloc(sizeof(char)*17);
  strcpy(hola,"SYSTEM UTNSO 0.1");

  /*
    5.  Ya conectados al servidor, vamos a hacer un handshake!
        Para esto, vamos a, primero recibir un mensaje del
        servidor y luego mandar nosotros un mensaje.
        Deberìamos recibir lo mismo que está contenido en la
        variable "hola". Entonces, vamos por partes:
        5.1.  Reservemos memoria para un buffer para recibir el mensaje.
  */
  char * buffer = malloc(sizeof(char)*16 + 1);
  /*
        5.2.  Recibamos el mensaje en el buffer.
        Recuerden el prototipo de recv:
        conexión - donde guardar - cant de bytes - flags(si no se pasa ninguno puede ir NULL)
        Nota: Palabra clave MSG_WAITALL.
  */
  int result_recv = recv(socket, buffer,17,MSG_WAITALL);
  /*
        5.3.  Chequiemos errores al recibir! (y logiemos, por supuesto)
        5.4.  Comparemos lo recibido con "hola".
              Pueden usar las funciones de las commons para comparar!
        No se olviden de loggear y devolver la memoria que pedimos!
        (si, también si falló algo, tenemos que devolverla, atenti.)
  */
  if(result_recv == 1){
        log_error(logger, "Error: al recibir mensaje hola");
        free(buffer);
        exit_gracefully(1);
  }
  log_info(logger, "Recibimos el dato OK");

  if(string_equals_ignore_case(buffer,hola)){
    log_info(logger, "Son iguales :)");
  }else{
    log_warning(logger, "No son iguales :(");
    puts(buffer);
  }

  free(buffer);
  free(hola);
}

Alumno read_hello() {
  /*
    6.    Ahora nos toca mandar a nosotros un mensaje de hola.
          que van a ser nuestros datos, definamos una variable de tipo Alumno.
          Alumno es esa estructura que definimos en el .h.
          Recuerden definir el nombre y apellido como cadenas varias, dado
          que como se va a enviar toda la estructura completa, para evitar problemas
          con otros otros lenguajes
  */
  Alumno alumno = { .nombre = "", .apellido = "" };

  /*
    7.    Pero como conseguir los datos? Ingresemoslos por consola!
          Para eso, primero leamos por consola usando la biblioteca realine.
          Vamos a recibir, primero el legajo, despues el nombre y
          luego el apellido
  */
  char * legajo = readline("Legajo: ");

  /*
    8.    Realine nos va a devolver un cacho de memoria ya reservada
          con lo que leyo del teclado hasta justo antes del enter (/n).
          Ahora, nos toca copiar el legajo al la estructura alumno. Como
          el legajo es numero, conviertanlo a numero con la funcion atoi
          y asignenlo.
          Recuerden liberar la memoria pedida por readline con free()!
  */

  alumno.legajo = atoi(legajo);
  free(legajo);

  /*
    9.    Para el nombre y el apellido no hace falta convertirlos porque
          ambos son cadenas de caracteres, por los que solo hace falta
          copiarlos usando memcpy a la estructura y liberar la memoria
          pedida por readline.
  */
  char * nombre = readline("Nombre: ");

  /*TODO: Tanto strcpy y memcpy solo COPIAN, ya sea cortando por el /0 o por la cantidad 
  de byte pero ninguno te reserva memoria, es decir si haces memcpy de un char*, previo
  es necesario un malloc, si es un char [10] no hace falta obviamente?*/
  memcpy(alumno.nombre , nombre , strlen(nombre) + 1);
  free(nombre);

  // Usemos memcpy(destino, origen, cant de bytes).
  // Para la cant de bytes nos conviene usar strlen dado que son cadenas
  // de caracteres que cumplen el formato de C (terminar en \0)

  // 9.1. Faltaría armar el del apellido
  char * apellido = readline("Apellido: ");
  
  memcpy(alumno.apellido,apellido,strlen(apellido) + 1 );
  free(apellido);

  // 10. Finalmente retornamos la estructura

  return alumno;
}

void send_hello(int socket, Alumno alumno) {
  log_info(logger, "Enviando info de Estudiante");
  /*
    11.   Ahora SI nos toca mandar el hola con los datos del alumno.
          Pero nos falta algo en nuestra estructura, el id_mensaje del protocolo.
          Segun definimos, el tipo de id para un mensaje de tamaño fijo con
          la informacion del alumno es el id 99
  */
  alumno.id_mensaje = 99;
  /*
    11.1. Como algo extra, podes probar enviando caracteres invalidos en el nombre
          o un id de otra operacion a ver que responde el servidor y como se
          comporta nuestro cliente.
  */  

  // alumno.id = 33;
  // alumno.nombre[2] = -4;

  /*
    12.   Finalmente, enviemos la estructura por el socket!
          Recuerden que nuestra estructura esta definida como __attribute__((packed))
          por lo que no tiene padding y la podemos mandar directamente sin necesidad
          de un buffer y usando el tamaño del tipo Alumno!
  */
  int resultado = send(socket, &alumno , sizeof(Alumno), 0);
  if(resultado == 1){
    log_error(logger, "Error: al tratar de enviar la estructura");
    close(socket);
    exit_gracefully(1);
  }
  /*
    12.1. Recuerden que al salir tenemos que cerrar el socket (ademas de loggear)!
  */
  
  log_info(logger, "Estructura enviado correctamente!!");
  
}

void * wait_content(int socket) {
  /*
    13.   Ahora tenemos que recibir un contenido de tamaño variable
          Para eso, primero tenemos que confirmar que el id corresponde al de una
          respuesta de contenido variable (18) y despues junto con el id de operacion
          vamos a haber recibido el tamaño del contenido que sigue. Por lo que:
  */

  log_info(logger, "Esperando el encabezado del contenido(%ld bytes)", sizeof(ContentHeader));
  // 13.1. Reservamos el suficiente espacio para guardar un ContentHeader
  
  ContentHeader * header = malloc(sizeof(ContentHeader));

  // 13.2. Recibamos el header en la estructura y chequiemos si el id es el correcto.
  //      No se olviden de validar los errores, liberando memoria y cerrando el socket!

  void* buffer_header = malloc(sizeof(int)*2);
  //tomo solo el header
  int recibo_header = recv(socket, buffer_header,sizeof(int)*2,MSG_WAITALL);
  if(recibo_header == 1){
    log_error(logger, "Error: al recibir el header");
    close(socket);
    exit_gracefully(1);  
  }

  //como se que es de dato variable, me sirve ahora si tener el len
  memcpy(&(header->id),buffer_header,sizeof(int));
  memcpy(&(header->len),buffer_header + sizeof(int),sizeof(int));

  if(header->id != 18){
    log_info(logger, "Error: recibimos un id para datos fijos");
    free(buffer_header);
    close(socket);
    exit_gracefully(1); 
  }
  log_info(logger, "Llego correctamente el header");
  log_info(logger, "Esperando el contenido (%d bytes)", header->len);
  free(buffer_header);

  /*
      14.   Ahora, recibamos el contenido variable. Ya tenemos el tamaño,
            por lo que reecibirlo es lo mismo que veniamos haciendo:
      14.1. Reservamos memoria
      14.2. Recibimos el contenido en un buffer (si hubo error, fallamos, liberamos y salimos
  */
  void* buffer_content = malloc(header->len);
  int recibo_content = recv(socket, buffer_content,header->len,MSG_WAITALL);

  if(recibo_content == 1){
    log_error(logger, "Error: al recibir el content");
    close(socket);
    free(buffer_content);
    exit_gracefully(1);  
  }

  log_info(logger, "Llego correctamente el contenido");
  /*
      15.   Finalmente, no te olvides de liberar la memoria que pedimos
            para el header y retornar el contenido recibido.
  */
  free(header);
  return buffer_content;
}

void send_md5(int socket, void * content) {
  /*
    16.   Ahora calculemos el MD5 del contenido, para eso vamos
          a armar el digest:
  */

  void * digest = malloc(MD5_DIGEST_LENGTH);
  MD5_CTX context;
  MD5_Init(&context);
  MD5_Update(&context, content, strlen(content) + 1);
  MD5_Final(digest, &context);

  free(content);

  /*
    17.   Luego, nos toca enviar a nosotros un contenido variable.
          A diferencia de recibirlo, para mandarlo es mejor enviarlo todo de una,
          siguiendo la logida de 1 send - N recv.
          Asi que:
  */

  //      17.1. Creamos un ContentHeader para guardar un mensaje de id 33 y el tamaño del md5


  //TODO: envio el MD5_DIGEST_LENGTH?? o como mido el tamaño posta del digest? + 1??
  int longitud = MD5_DIGEST_LENGTH;
  ContentHeader header = { .id=33 , .len=longitud  };

  /*
          17.2. Creamos un buffer del tamaño del mensaje completo y copiamos el header y la info de "digest" allí.
          Recuerden revisar la función memcpy(ptr_destino, ptr_origen, tamaño)!
  */
  void * buffer = malloc(sizeof(ContentHeader) + header.len);
  //Aclaracion: el header se manda asi por ser una struct fijo
  memcpy(buffer,&header,sizeof(ContentHeader));
  memcpy(buffer + sizeof(ContentHeader),digest, header.len);
  /*
    18.   Con todo listo, solo nos falta enviar el paquete que armamos y liberar la memoria que usamos.
          Si, TODA la que usamos, eso incluye a la del contenido del mensaje que recibimos en la función
          anterior y el digest del MD5. Obviamente, validando tambien los errores.
  */
  int envio = send(socket, buffer , (sizeof(ContentHeader) + header.len), 0);
  if(envio == 1){
    log_error(logger, "Error: al tratar de enviar la el buffer con header + digest");
    close(socket);
    exit_gracefully(1);
  }
  free(digest);
  free(buffer);

}

void wait_confirmation(int socket) {
  int result = 1; // Dejemos creado un resultado por defecto
  /*
    19.   Ahora nos toca recibir la confirmacion del servidor.
          Si el resultado obvenido es distinto de 0, entonces hubo un error
  */
  
  int confirmacion = recv(socket,&result,sizeof(int),0);
  if(confirmacion == 1){
    log_error(logger, "Error: al tratar de recibir el resultado de mandar el buffer");
    close(socket);
    exit_gracefully(1);
  }

  if (result != 0){
    log_error(logger, "Error: los MD5 no coinciden");
    close(socket);
    exit_gracefully(1);
  }
  log_info(logger, "Los MD5 concidieron!");
  close(socket);
}

void exit_gracefully(int return_nr) {
  /*
    20.   Siempre llamamos a esta funcion para cerrar el programa.
          Asi solo necesitamos destruir el logger y usar la llamada al
          sistema exit() para terminar la ejecucion
  */
  log_destroy(logger);
  exit(return_nr);
}
