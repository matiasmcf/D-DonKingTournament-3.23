#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/wait.h>
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>

int obtenerConexion(int *sockfd) 
{ 
 
 int tr=1; /*Entero que uso para setsockopt*/

 /* Creamos un socket orientado a conexion */
 if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
 { 
   perror("socket"); 
   return 1; 
 }

 /* Definimos el socket para ser reutilizable */
setsockopt(*sockfd, SOL_SOCKET,SO_REUSEADDR,&tr,sizeof(tr));

return 0;

}

int asociarConexion(int *sockfd, int puerto)
{
 
 struct sockaddr_in my_addr;  /* contendra la direccion IP y el numero 					   de puerto local */ 
 /* Asignamos valores a la estructura my_addr para luego poder llamar a  la funcion bind() */ 
 my_addr.sin_family = AF_INET; /*no debe convertirse a  network byte 					order, es solo utilizado por el kernel*/ 
 my_addr.sin_port = htons(puerto); /*debe convertirse a network byte 					   order porque es enviado por la red*/ 
 my_addr.sin_addr.s_addr = INADDR_ANY;    /* Cualquier direccion para 							recibir */ 
 bzero(&(my_addr.sin_zero), 8);  /* rellena con ceros el resto de la  estructura */ 
 /* Le asignamos un nombre al socket */ 
 if ( bind(*sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
 {
   perror("bind"); 
   return 1;
 }

return 0;

}

int escucharConexiones(int *sockfd, int backlog)
{
 /* Habilitamos el socket para recibir conexiones, con una cola de backlog conexiones en espera como maximo */ 
 if (listen(*sockfd, backlog) == -1)
 { 
   perror("listen"); 
   return 1;
 }

return 0;

}


int aceptarConexion(int *sockfd, int *newfd)
{

 struct sockaddr_in their_addr;   /* Contendra la direccion IP y numero 						de puerto del cliente */ 
 socklen_t sin_size;  /* Contendra el tamanio de la estructura sockaddr_in */ 

 sin_size = sizeof(struct sockaddr_in); 

  /* Esperamos por conexiones */ 
  if ((*newfd = accept(*sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
  {
	perror("accept"); 
	return 1;
  }

 return 0;
  
} 

int conectar(int *sockfd, char *ipServidor, int puerto)
{
  struct sockaddr_in srv_addr;
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(puerto);
  inet_aton(ipServidor,&srv_addr.sin_addr);

  bzero(&(srv_addr.sin_zero), 8); //completo con ceros//

  if(connect(*sockfd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr)) == -1)
	{
		//perror("connect");
		return 1;
	}

return 0;

}

int recibirDatos(int *sockfd,void *datos, int tamanio)
{
    /* Se reciben los datos */ 
    return recv(*sockfd, datos, tamanio, MSG_WAITALL);
}

int recibirDatosNoBloqueante(int *sockfd,void *datos, int tamanio)
{
    /* Se reciben los datos */ 
    return recv(*sockfd, datos, tamanio, MSG_DONTWAIT);
}

int enviarDatos(int *sockfd,void *datos, int tamanio)
{
    /* Enviamos respuesta al Cliente */
    return send(*sockfd, datos, tamanio, MSG_DONTWAIT);
}

int terminarConexion(int *sockfd)
{
  return close(*sockfd);
}
