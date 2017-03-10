/* Sistemas Operativos - Trabajo Practico 4
   Comision: Lunes y Miercoles de 8 a 12hs
   Tipo: Entrega
   Integrantes:
   FACUNDO
   ARIEL
   AGUSTIN
   MATIAS

   RODRIGUEZ VIANO, Sebastian. DNI: 35.361.326
	 */

//INCLUDES
	#include <stdio.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <sys/shm.h>
	#include <signal.h>
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <string.h>
	#include <sys/msg.h>
	#include <pthread.h>
	#include <stdio.h>
	#include <semaphore.h>
    #include <sys/ipc.h>
   	#include <fcntl.h>
	#include "./socketsLib/socketsLib.h"
	#include "./semaforosLib/semaforos.h"
	#include <X11/Xlib.h>

//DEFINES

   //COMUNICACIÓN DE EVENTOS
		#define SIN_CAMBIOS 0
		#define GOLPEADO_POR_LLAMA 1
		#define GOLPEADO_POR_BARRIL 2
		#define RESCATO_PRINCESA 3
		#define TIENE_MARTILLO 4
		#define GANO_LA_PARTIDA 5
   		#define PERDIO_LA_PARTIDA 6

   	//OTROS

		#define LONG_RECORRIDO_BARRIL 6

//STRUCTS
	typedef struct  
	{               
		pid_t id_serverPartida;
		int nroJugador1;
		int evento1;
		int nroJugador2;
		int evento2;
		int socketGanador;
	}t_paquete_serv;

 	typedef struct 
	{
		int numero;
		char nombre[10];
		int recorrerllama1[5];
		int recorrerllama2[5];
		int recorrerllama3[5];
		int recorrerllama4[5];
		int recorrerllama5[5];
		int cantRescates;
	}t_identificacion;

	typedef struct
	{
		int numero;
		int x;
		int y;
		int estado;
		int barril;
		int recorrido[6];
		int llama;	//Si hay un 1: llegó un barril //Si hay un 2 creo una llama
		int frameActual;
		int sentidoLlama;
	}t_movimientos;

	typedef struct
	{
		int nroJugador;
		int nombre;
		int princesasRescatadas;
		int vidasPerdidas;
	}t_info_jugador;

//PROTOTIPOS
	void generarRecorridoBarril(t_movimientos*, t_movimientos*);
	void generarRecorridosLlamas(t_identificacion*,t_identificacion*);
	void *vaciarSocket(void *param);
	void eliminarPID();

//VARIABLES GLOBALES
	int ID_PARTIDA;
	int RESCATES=0;
	int DIFICULTAD=0;
	t_info_jugador infoJugador[2];
	pid_t demonio; //pid del proceso demonio
	int clifd1, clifd2,shmid, longproceso, msqid;
	int nroCliente1, nroCliente2;
	pthread_t hilo[5]; //pids de los threads
	t_paquete_serv paqserv, *buffer=NULL;
	sem_t *mtxTorneoPartida=NULL;
	FILE *fpid=NULL;
	int fifo; //Identificador del archivo para la comunicación entre servidores
	char DIR_PID[50],DIR_AUX_PID[50], nom_arch[20], comando[50];
	key_t CLAVE, CLAVEC;
	//pthread_mutex_t recibemtx=PTHREAD_MUTEX_INITIALIZER;

void eliminarPID()
  {
  	printf("PARTIDA: ELIMINANDO PID\n");
  	//pedirSemaforo(mtxPIDS);
    char comando2[50], comando3[50];
    sprintf(comando2,"./.PID/%d",demonio);
    remove(comando2);
    printf("PARTIDA: ELIMINÉ MI PID\n");
    //devolverSemaforo(mtxPIDS);
  }

void handler(int iNumSen,siginfo_t *info,void *ni) //funcion declarada para el
{                //manejo de las senales recibidas por el proceso demonio.
	//int ERRStream = open("/dev/null", O_WRONLY);
	switch(iNumSen)
	{
		case SIGINT: //respuesta a la senal SIGINT
						//dup2(STDERR_FILENO,ERRStream);
					  	signal(SIGINT,SIG_IGN);
					  	printf("SERVIDOR DE PARTIDA: Se canceló el programa serverPartida PID: %d (SIGINT)\n",getpid());
					  	raise(SIGTERM); //se autoenvia la senal SIGTERM para que finalice
					  	break;               //correctamente el proceso
		 
		case SIGTERM: //caso de finalizacion del programa 
						//dup2(STDERR_FILENO,ERRStream);
						int x;
						sleep(3);
						for(x=0;x<1;x++)
							pthread_cancel(hilo[x]);
						//eliminarPID();
						shmdt((char*)buffer);
						if(info->si_pid==getpid())
							printf("SERVIDOR DE PARTIDA [%d]: Finalizando\n",ID_PARTIDA);
						else	
							printf("SERVIDOR DE PARTIDA [%d]: Se finalizó el programa. (SIGTERM)\n",ID_PARTIDA);
						exit(0); 
						break;  //se finaliza el proc. demonio.
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void *manejoDeMensajes(void *param)
{
	srand(time(NULL));
	t_identificacion paq, paq2;
	t_movimientos movimientos;
	t_movimientos movimientos2;
	int auxSentido;
	int valor_p;
	int recorrido[6];
	bzero(&movimientos,sizeof(t_movimientos));
	bzero(&movimientos2,sizeof(t_movimientos));
	bzero(&paq,sizeof(t_identificacion));
	if(recibirDatos(&clifd1,&paq,sizeof(t_identificacion))==-1)
	{
		printf("SERVIDOR DE PARTIDA [%d]: Error al recibir paquete de datos del Cliente 1.\n",ID_PARTIDA);
		kill(demonio,SIGTERM);
		//raise(SIGTERM);
	}
	//printf("Recibí: %s\n",paq.nombre);
	paq.numero=1;
	movimientos.numero=0;
	bzero(&paq2,sizeof(t_identificacion));
	if(recibirDatos(&clifd2,&paq2,sizeof(t_identificacion))==-1)
	{
		printf("SERVIDOR DE PARTIDA [%d]: Error al recibir paquete de datos del Cliente 2.\n",ID_PARTIDA);
		kill(demonio,SIGTERM);
		//raise(SIGTERM);
	}
	//printf("Recibí: %s\n",paq2.nombre);
	generarRecorridosLlamas(&paq,&paq2);
	paq.cantRescates=paq2.cantRescates=RESCATES;
	if(enviarDatos(&clifd2,&paq,sizeof(t_identificacion))==-1)
	{
		printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
		kill(demonio,SIGTERM);
	}
	paq2.numero=0;
	movimientos2.numero=1;
	if(enviarDatos(&clifd1,&paq2,sizeof(t_identificacion))==-1)
	{
		printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
		kill(demonio,SIGTERM);
	}
	printf("SERVIDOR DE PARTIDA [%d]: Información inicial enviada a %d y %d\n",ID_PARTIDA,clifd1,clifd2);
	// //
	// bzero(buffer,sizeof(t_paquete_serv));
	// 	printf("PARTIDA BUFFER:\n\tid_partida: %d\tsocketGanador: %d\n\tevento1: %d\t evento2: %d\n\tnroJugador1: %d\tnroJugador2: %d\n",buffer->id_serverPartida,buffer->socketGanador,buffer->evento1,buffer->evento2,buffer->nroJugador1,buffer->nroJugador2);
	// //
	int abandono=0;
	int victoria=0;
	int ganador=0;
	while(ganador==0)
 	{
		//Recibo c1
		bzero(&movimientos,sizeof(t_movimientos));
		if(recibirDatos(&clifd1,&movimientos,sizeof(t_movimientos))==-1)
		{
			eliminarPID();
			bzero(&movimientos,sizeof(t_movimientos));
			printf("SERVIDOR DE PARTIDA [%d]: Error al recibir paquete de datos del Cliente 1.\nSe considerará como abandono\n",ID_PARTIDA);
			if(enviarDatos(&clifd2,&movimientos,sizeof(t_movimientos))==-1)
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
			pedirSemaforo(mtxTorneoPartida);
			buffer->id_serverPartida=ID_PARTIDA;
			buffer->evento2=GANO_LA_PARTIDA;
			buffer->socketGanador=nroCliente2;
			devolverSemaforo(mtxTorneoPartida);
			abandono=1;
			valor_p = pthread_create(&hilo[1],NULL,vaciarSocket,(void*)&clifd2);
		   	if(valor_p)
		   	{
				printf("SERVIDOR DE PARTIDA [%d]: Error en la creacion de thread de recepcion de mensajes\n",ID_PARTIDA);
				kill(getpid(),SIGTERM);
		   	}
			sleep(1);
			pthread_cancel(hilo[1]);
			kill(demonio,SIGTERM);
			//raise(SIGTERM);
		}
		//Recibo c2
		bzero(&movimientos2,sizeof(t_movimientos));
		if(recibirDatos(&clifd2,&movimientos2,sizeof(t_movimientos))==-1)
		{
			eliminarPID();
			bzero(&movimientos2,sizeof(t_movimientos));
			printf("SERVIDOR DE PARTIDA [%d]: Error al recibir paquete de datos del Cliente 2.\nSe considerará como abandono\n",ID_PARTIDA);
			if(enviarDatos(&clifd1,&movimientos2,sizeof(t_movimientos))==-1)
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 1.\n",ID_PARTIDA);
			pedirSemaforo(mtxTorneoPartida);
			buffer->id_serverPartida=ID_PARTIDA;
			buffer->evento1=GANO_LA_PARTIDA;
			buffer->socketGanador=nroCliente1;
			devolverSemaforo(mtxTorneoPartida);
			abandono=1;
			valor_p = pthread_create(&hilo[1],NULL,vaciarSocket,(void*)&clifd1);
		   	if(valor_p)
		   	{
				printf("SERVIDOR DE PARTIDA [%d]: Error en la creacion de thread de recepcion de mensajes\n",ID_PARTIDA);
				kill(getpid(),SIGTERM);
		   	}
			sleep(1);
			pthread_cancel(hilo[1]);
			kill(demonio,SIGTERM);
			//raise(SIGTERM);
		}
		//SI HUBO UN ABANDONO
		// if(abandono==1)
		// {
		// 	sleep(1);
		// 	//printf("FINALIZANDO PARTIDA %d\n",ID_PARTIDA);
		// 	kill(getpid(),SIGTERM);
		// }

		//VERIFICO EL ESTADO DE LOS JUGADORES
		switch(movimientos.estado)
		{
			case SIN_CAMBIOS:
					break;
			case GOLPEADO_POR_LLAMA:
					infoJugador[0].vidasPerdidas++;
					break;
			case GOLPEADO_POR_BARRIL:
					infoJugador[0].vidasPerdidas++;
					break;
			case RESCATO_PRINCESA:
					infoJugador[0].princesasRescatadas++;
					break;
			case TIENE_MARTILLO:
					break;
			case GANO_LA_PARTIDA:
					break;
		}
		switch(movimientos2.estado)
		{
			case SIN_CAMBIOS:
					break;
			case GOLPEADO_POR_LLAMA:
					infoJugador[1].vidasPerdidas++;
					break;
			case GOLPEADO_POR_BARRIL:
					infoJugador[1].vidasPerdidas++;
					break;
			case RESCATO_PRINCESA:
					infoJugador[1].princesasRescatadas++;
					break;
			case TIENE_MARTILLO:
					break;
			case GANO_LA_PARTIDA:
					break;
		}

		//VERIFICAMOS SI HAY UN GANADOR
		if(infoJugador[0].princesasRescatadas==RESCATES)
		{
			ganador=1;
			eliminarPID();
			movimientos.estado=PERDIO_LA_PARTIDA;
			if(enviarDatos(&clifd2,&movimientos,sizeof(t_movimientos))==-1)
			{
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
				kill(demonio,SIGTERM);
			}
			movimientos2.estado=GANO_LA_PARTIDA;
			if(enviarDatos(&clifd1,&movimientos2,sizeof(t_movimientos))==-1)
			{
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 1.\n",ID_PARTIDA);
				kill(demonio,SIGTERM);
			}
			pedirSemaforo(mtxTorneoPartida);
			buffer->id_serverPartida=ID_PARTIDA;
			buffer->evento1=GANO_LA_PARTIDA;
			buffer->socketGanador=nroCliente1;
			devolverSemaforo(mtxTorneoPartida);
			valor_p = pthread_create(&hilo[1],NULL,vaciarSocket,(void*)&clifd2);
		   	if(valor_p)
		   	{
				printf("SERVIDOR DE PARTIDA [%d]: Error en la creacion de thread de recepcion de mensajes\n",ID_PARTIDA);
				kill(getpid(),SIGTERM);
		   	}
			sleep(2);
			pthread_cancel(hilo[1]);
			kill(demonio,SIGTERM);
		}
		else
			if(infoJugador[1].princesasRescatadas==RESCATES)
				{
					eliminarPID();
					ganador=1;
					movimientos2.estado=PERDIO_LA_PARTIDA;
					if(enviarDatos(&clifd1,&movimientos2,sizeof(t_movimientos))==-1)
					{
						printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 1.\n",ID_PARTIDA);
						kill(demonio,SIGTERM);
					}
					movimientos.estado=GANO_LA_PARTIDA;
					if(enviarDatos(&clifd2,&movimientos,sizeof(t_movimientos))==-1)
					{
						printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
						kill(demonio,SIGTERM);
					}
					pedirSemaforo(mtxTorneoPartida);
					buffer->id_serverPartida=ID_PARTIDA;
					buffer->evento2=GANO_LA_PARTIDA;
					buffer->socketGanador=nroCliente2;
					devolverSemaforo(mtxTorneoPartida);
					valor_p = pthread_create(&hilo[1],NULL,vaciarSocket,(void*)&clifd1);
				   	if(valor_p)
				   	{
						printf("SERVIDOR DE PARTIDA [%d]: Error en la creacion de thread de recepcion de mensajes\n",ID_PARTIDA);
						kill(getpid(),SIGTERM);
				   	}
					sleep(2);
					pthread_cancel(hilo[1]);
					kill(demonio,SIGTERM);
				}
		if(ganador!=1)
		{
			//INFORMAMOS EL ESTADO DE LA PARTIDA AL SERVIDOR DEL TORNEO
			pedirSemaforo(mtxTorneoPartida);
			buffer->id_serverPartida=ID_PARTIDA;
			buffer->evento1=movimientos.estado;
			buffer->nroJugador1=movimientos.numero;
			buffer->evento2=movimientos2.estado;
			buffer->nroJugador2=movimientos2.numero;
			devolverSemaforo(mtxTorneoPartida);
			if(movimientos.estado!=5 && movimientos.estado!=6)
				movimientos.estado=0;
			if(movimientos2.estado!=5 && movimientos2.estado!=6)
				movimientos2.estado=0;
			//VEO SI CREAR BARRILES O LLAMAS
			if(rand()%(250-(DIFICULTAD)*100)<3)
			{
				generarRecorridoBarril(&movimientos,&movimientos2);
				movimientos.barril=1;
				movimientos2.barril=1;
			}
			if(movimientos.llama==1||movimientos2.llama==1)
			{
				if(rand()%100<7*(DIFICULTAD+1))
				{
					movimientos.llama=2;
					movimientos2.llama=2;
					if(auxSentido=rand()%2==0)
						auxSentido=1;
					else
						auxSentido=-1;
					movimientos.sentidoLlama=auxSentido;
					movimientos2.sentidoLlama=auxSentido;
				}					
			}
			//ENVÍO LOS DATOS A LOS CLIENTES
			if(enviarDatos(&clifd2,&movimientos,sizeof(t_movimientos))==-1)
			{
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 2.\n",ID_PARTIDA);
				eliminarPID();
				kill(demonio,SIGTERM);
			}
			if(enviarDatos(&clifd1,&movimientos2,sizeof(t_movimientos))==-1)
			{
				printf("SERVIDOR DE PARTIDA [%d]: Error al enviar paquete de datos a Cliente 1.\n",ID_PARTIDA);
				eliminarPID();
				kill(demonio,SIGTERM);
			}
		}
	}  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) //main. Programa.
{
	XInitThreads();//
	int valor_p=0;
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	signal(SIGUSR1,SIG_IGN); //SIGUSR1 para la finalizacion correcta de threads
	signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
	signal(SIGUSR2,SIG_IGN);
	mtxTorneoPartida=obtenerMutex("/mtxTorneoPartida");
	for(int i=0;i<5;i++)
		hilo[i]=0;

	//Inicializamos la estructura de control de la partida
	infoJugador[0].nroJugador=0;
	infoJugador[1].nroJugador=1;
	infoJugador[0].vidasPerdidas=infoJugador[1].vidasPerdidas=infoJugador[0].princesasRescatadas=infoJugador[1].princesasRescatadas=0;

  	if((fork())==0) //creacion del proceso demonio
  	{
  		demonio=getpid();
		act.sa_sigaction=handler; //acciones correspondientes al manejo de la 
		sigfillset(&act.sa_mask); //estructura sigaction. 
		act.sa_flags=SA_SIGINFO; //indicacion de uso de la funcion handler
					 //SIGINT y SIGTERM. Demas senales
		sigaction(SIGTERM,&act,NULL); //se trabajan con su respuesta por defecto.
		sigaction(SIGINT,&act,NULL);
		signal(SIGUSR1,SIG_IGN); //SIGUSR1 para la finalizacion correcta de threads
		signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
		signal(SIGUSR2,SIG_IGN);
		sleep(1); //suspension de proceso(para matar a su padre y asi quedar 
		sprintf(nom_arch,"%d",getpid());
   		sprintf(DIR_PID,"./.PID/%s",nom_arch);
   		sprintf(DIR_AUX_PID,"./.aux/%s2",nom_arch);

   		//ESCRIBIMOS LA INFORMACIÓN PARA EL PROCESO MONITOR
   		int flagIP=0;
   		fpid=fopen(DIR_PID,"a");
		if(!fpid)
   		{
   			printf("\nError al crear archivo de PID. Fin del proceso Cliente donkey.\n");
   			exit(1);
   		}
   		FILE * fp = popen("ifconfig", "r");
        if (fp) 
        {
                char *p=NULL, *e; size_t n;
                while ((getline(&p, &n, fp) > 0) && p && flagIP==0) 
                {
                   if (p = strstr(p, "inet ")) 
                   {
                        p+=5;
                        if (p = strchr(p, ':')) 
                        {
                            ++p;
                            if (e = strchr(p, ' ')) 
                            {
                                 *e='\0';
                                 fprintf(fpid,"%s\n", p);
                                 flagIP=1;
                            }
                        }
                   }
              }
        }
        fprintf(fpid,"%s\n", "partida");
        pclose(fp);
   		fclose(fpid);
		//Memoria compartida
		CLAVE=ftok(".",'M');
		if(CLAVE==-1)
		{
			printf("SERVIDOR DE PARTIDA: Error al intentar crear clave de memoria compartida. Fin\n");
			exit(0);
		}
		if((shmid=shmget(CLAVE, sizeof(t_paquete_serv), 0660))==-1)
		{
			printf("SERVIDOR DE PARTIDA: Error al intentar obtener ID de memoria compartida. Fin. server Partida\n");
			exit(0);
		}
		if((buffer=(t_paquete_serv*)shmat(shmid,(char*)0, 0))==NULL)
		{
			printf("SERVIDOR DE PARTIDA: Error en attach de segmento de memoria compartida. Fin. server Partida \n");
			exit(1);
		}
		//
					//como demonio.
		clifd1=atoi(argv[1]);
		clifd2=atoi(argv[2]);
		ID_PARTIDA=atoi(argv[3]);
		nroCliente1=atoi(argv[4]);
		nroCliente2=atoi(argv[5]);
		RESCATES=atoi(argv[6]);
		DIFICULTAD=atoi(argv[7]);
		char auxDif[12];
		switch(DIFICULTAD)
		{
			case 0: strcpy(auxDif,"Brasil");break;
			case 1: strcpy(auxDif,"Normal");break;
			case 2: strcpy(auxDif,"Extreme");break;
		}
		printf("\nINICIANDO SERVIDOR DE PARTIDA [PID: %d\tID: %d]\n",getpid(),ID_PARTIDA);
		printf("DIFICULTAD: %s\nRESCATES: %d\nNRO JUGADORES: %d vs %d\nSOCKETS: %d vs %d\n",auxDif,RESCATES,nroCliente1,nroCliente2,clifd1,clifd2);
	 	valor_p = pthread_create(&hilo[0],NULL,manejoDeMensajes,NULL);
	   	if(valor_p)
	   	{
			printf("SERVIDOR DE PARTIDA [%d]: Error en la creacion de hilos(threads) de recepcion de mensajes. Error devuelto: %d\n",ID_PARTIDA, valor_p);
			kill(getpid(),SIGTERM);
	   	}
	  	while(1) //bucle infinito del proceso demonio. Aguarda por una senal enviada por el usuario
	  	{			//todo el trabajo lo realizan los threads
	  		pause();       
	   	}
  	}
  	else
  	{
   		if(demonio==-1)
   		{
			printf("Error al crear proceso Server Partida(Demonio). serverPartida PID: %d. Fin\n", getpid());
			exit(0); 
   		}
  	}
 	exit(0); //muerte del proceso padre principal. Fuerza al hijo a quedar como demonio
}

////////////////////////////////////////////////////////////////////////////////////////

void generarRecorridoBarril(t_movimientos* m1,t_movimientos*m2)
{
	for (int x=0;x<LONG_RECORRIDO_BARRIL;x++)
		m1->recorrido[x]=m2->recorrido[x]=rand()%2;
}

////////////////////////////////////////////////////////////////////////////////////////

void generarRecorridosLlamas(t_identificacion*id,t_identificacion*id2)
{
	int auxRecorrido=0;
	for (int x=0;x<LONG_RECORRIDO_BARRIL-1;x++)
	{
		auxRecorrido=rand()%2;
		id->recorrerllama1[x]=auxRecorrido;
		id2->recorrerllama1[x]=auxRecorrido;
	}
	for (int x=0;x<LONG_RECORRIDO_BARRIL-1;x++)
	{
		auxRecorrido=rand()%2;
		id->recorrerllama2[x]=auxRecorrido;
		id2->recorrerllama2[x]=auxRecorrido;
	}
	for (int x=0;x<LONG_RECORRIDO_BARRIL-1;x++)
	{
		auxRecorrido=rand()%2;
		id->recorrerllama3[x]=auxRecorrido;
		id2->recorrerllama3[x]=auxRecorrido;
	}
	for (int x=0;x<LONG_RECORRIDO_BARRIL-1;x++)
	{
		auxRecorrido=rand()%2;
		id->recorrerllama4[x]=auxRecorrido;
		id2->recorrerllama4[x]=auxRecorrido;
	}
	for (int x=0;x<LONG_RECORRIDO_BARRIL-1;x++)
	{
		auxRecorrido=rand()%2;
		id->recorrerllama5[x]=auxRecorrido;
		id2->recorrerllama5[x]=auxRecorrido;
	}
}

/************************************************************************************************/

void *vaciarSocket(void *param)
{
	int *socket=(int*)param;
	t_movimientos aux;
	//printf("Limpiando socket %d\n",*socket);
	while(1)
	{
		recibirDatosNoBloqueante(socket,&aux,sizeof(t_movimientos));
	}
}