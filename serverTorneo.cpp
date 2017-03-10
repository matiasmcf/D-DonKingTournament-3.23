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
	#include <unistd.h>
	#include <sys/shm.h>
	#include <sys/ipc.h>
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
	#include <SDL/SDL.h>
	#include <SDL/SDL_ttf.h>
	#include <semaphore.h>
	#include <fcntl.h>
	#include <X11/Xlib.h>
	#include "./socketsLib/socketsLib.h"
	#include "./semaforosLib/semaforos.h"

//MACROS

	//PANTALLA
		#define ALTO 400
		#define ANCHO 800

		#define FONT "./fonts/arlrdbd.ttf"

//PROTOTIPOS DE FUNCIONES
	void create_label(int , int , const char *, SDL_Color, TTF_Font*);
	void cargarEsperaEnPantalla(int );
	void borrarSectorDePantalla(int , int , int , int );
	void cargarCantidadDeConectadosEnPantalla(int );
	void iniciarSDL();
	void informarEnPantalla();
	int buscarPidServidor(int);
	void asignarPartidas();
	void eliminarPID();
	int cargarConfiguracion(const char*);

	void* verificarEstadoDePartidas(void*param);
	void* lanzarProximaRonda(void*param);
	void* SDLtorneo(void*param);

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
		int clifd,servfd;
		pid_t pid;
	}t_paquete_cliente;

	typedef struct
	{
		char nombre[15];
		int clifd;
	}t_datos_cliente;

	typedef struct
	{
		char nombre1[15];
		char nombre2[15];
		int clifd[2];
		int rescates[2];
		int vidasPerdidas[2];
		int clienteGanador;
	}t_datos_partida;

	typedef struct
	{
		char nombre1[15];
		char nombre2[15];
		char ganador[15];
	}t_partida_finalizada;

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

//VARIABLES GLOBALES

	//Configuración del servidor
	int DIFICULTAD=0;
	int RESCATES=0;
	int TIEMPO_INSCRIPCION=0;
	int PUERTO=0;
	//
	int ASIGNANDO_PARTIDA=0;
	int CANT_GANADORES=0;
	int RONDA_ACTUAL=0;
	int FINALIZAR_TORNEO=0;
	//
	int tiempo=0;
	char campeon[20];
	int HAY_CAMPEON=0;
	//COLORES SDL
	SDL_Color BLANCO={255,255,255};
	SDL_Color ROJO={255,0,0};
	SDL_Color AZUL={0,0,255};
	SDL_Color VERDE={0,255,0};
	SDL_Color CELESTE={0,162,232};
	//
	TTF_Font *fuente;
	TTF_Font *fuenteVictoria;
	pid_t demonio, serverP[10]; //pid del proceso demonio
	int servfd,shmid,longcliente, clientesConectados=0, servidoresPartida=0, cantThreads=0, cantPartidasFinalizadas=0;;
	pthread_t hilo[5]; //pids de los threads
	t_paquete_cliente cliente[20],paquete;
	t_datos_partida datosPartida[20];
	t_datos_cliente datosCliente[20];
	t_partida_finalizada partidasFinalizadas[20];
	t_paquete_serv paqserv;
	t_paquete_serv *buffer=NULL;
	sem_t *mtxmonitor=NULL; //puntero a semaforos nombrados que se utilizaran para relanzar el proceso monitor
	FILE *fpid=NULL;
	char DIR_PID[50],DIR_AUX_PID[50], nom_arch[20], comando[50];
	key_t CLAVE; //calves memoria compartida y cola de mensajes
	 //semaforos mutex a utilizar en los threads
	//pthread_mutex_t buffmtx=PTHREAD_MUTEX_INITIALIZER;
	//pthread_mutex_t recibemtx=PTHREAD_MUTEX_INITIALIZER;
	SDL_Surface *pantalla;
	int fifo;
	sem_t *mtxTorneoPartida=NULL;

/**************************************************************************************/

void eliminarPID()
	{
		char comando2[50], comando3[50];

        sprintf(comando2,"./.PID/%s",nom_arch);
        //sprintf(comando3,"mv ./.aux/%s2 ./.PID/%s",nom_arch, nom_arch);
        remove(comando2);
        //printf("TORNEO: ELIMINÉ MI PID\n");
        //devolverSemaforo(mtxPIDS);
	}

///////////////////////////////////////////////////////////////////////////////////////

TTF_Font* cargarFuente(const char*path, int tam_fuente)
{
		TTF_Font *fuente;
		fuente = TTF_OpenFont(path, tam_fuente);
		if(fuente==NULL)
		{
			fprintf(stderr, "Error al cargar la fuente: %s\n",TTF_GetError());
			exit(1);
		}
		return fuente;
}

/*************************************************************************************/

void handler(int iNumSen,siginfo_t *info,void *ni) //funcion declarada para el
{                //manejo de las senales recibidas por el proceso demonio.
	//int ERRStream = open("/dev/null", O_WRONLY);
 switch(iNumSen)
  {

  case SIGINT: //respuesta a la senal SIGINT
  				//dup2(STDERR_FILENO,ERRStream);
              signal(SIGINT,SIG_IGN);
              printf("\nSe aborto el programa.(SIGINT)\n");
              raise(SIGTERM); //se autoenvia la senal SIGTERM para que finalice
              break;               //correctamente el proceso
 
  case SIGTERM: //caso de finalizacion del programa 
  				//sleep(3);
  				//dup2(STDERR_FILENO,ERRStream);
				int x;
				for(x=0;x<cantThreads;x++)
					pthread_cancel(hilo[x]);
				// for(x=0;x<servidoresPartida;x++)
				// 	kill(serverP[x], SIGTERM);
				
				//pthread_mutex_destroy(&buffmtx);		
				
				for(x=0;x<clientesConectados;x++)
		            terminarConexion(&cliente[x].clifd);

				//if(servfd)		
				terminarConexion(&servfd);

				if(shmctl(shmid,IPC_RMID,0)==-1)
					printf("\nError al intentar eliminar memoria compartida.");	

				shmdt((char*)buffer);

		     	eliminarSemaforo(mtxmonitor, "/mtxmonitor");
		     	eliminarSemaforo(mtxTorneoPartida,"/mtxTorneoPartida");    
		     	if(info->si_pid==getpid())
					printf("FIN DEL PROGRAMA\n");
				else
					printf("Se finalizo el programa (SIGTERM).\n");  
		        exit(0); 
		        break;  //se finaliza el proc. demonio.
   }
}

//////////////////////////////THREADS//////////////////////////////

void *obtener_Y_AceptarConexiones(void *param)
{
	int y=0;
	if(obtenerConexion(&servfd)==1)
	{
		printf("\nError al obtener conexion de socket server. Abortando programa\n");
		kill(getpid(),SIGTERM);
	}
	if(asociarConexion(&servfd,PUERTO)==1)
	{
		printf("\nError al asociar de socket server. Abortando programa\n");
		kill(getpid(),SIGTERM);
	}
	if(escucharConexiones(&servfd,5)==1)
	{
		printf("\nError al escuchar conexiones de socket server. Abortando programa\n");
		kill(getpid(),SIGTERM);
	}
	t_paquete_cliente aux;
  	while(1)
  	{
  				if(aceptarConexion(&servfd,&aux.clifd)==1)
				{
					printf("\nError al aceptar conexion de Cliente Nro %d. Abortando programa\n", clientesConectados);
					kill(getpid(),SIGTERM);
				}
  		// 	if(aceptarConexion(&servfd,&cliente[clientesConectados].clifd)==1)
				// {
				// 	printf("\nError al aceptar conexion de Cliente Nro %d. Abortando programa\n", clientesConectados+1);
				// 	kill(getpid(),SIGTERM);
				// }
  			if(tiempo>0)
	        {
	        	cliente[clientesConectados].clifd=aux.clifd;
				t_identificacion paquete;
		      	paquete.numero=0;
		      	bzero(&paquete, sizeof(t_identificacion));
				if(recibirDatos(&cliente[clientesConectados].clifd,&paquete,sizeof(t_identificacion))==-1)
				{
		        	printf("\nError al recibir paquete de datos del Cliente %d\n",clientesConectados);
		        	kill(getpid(),SIGTERM);
		        }
				char cad[200];
				bzero(cad,200);
				sprintf(cad,"%s se ha conectado\0",paquete.nombre);
				create_label(15,45+15*y,cad,BLANCO,fuente);
				y++;
				//Guardamos la infirmación del cliente
				strcpy(datosCliente[clientesConectados].nombre,paquete.nombre);
				datosCliente[clientesConectados].clifd=cliente[clientesConectados].clifd;
				//strcpy(datosPartida[clientesConectados].nombre1,paquete.nombre);
				//strcpy(datosPartida[clientesConectados].nombre2,paquete.nombre);
				// datosPartida[clientesConectados].rescates[0]=datosPartida[clientesConectados].rescates[1]=0;
				// datosPartida[clientesConectados].vidasPerdidas[0]=datosPartida[clientesConectados].vidasPerdidas[1]=0;
				// datosPartida[clientesConectados].clienteGanador=-1;
				clientesConectados++;
			}
			else
			{
				t_identificacion paquete;
				paquete.numero=-1;
		      	//bzero(&paquete, sizeof(t_identificacion));
				if(recibirDatos(&aux.clifd,&paquete,sizeof(t_identificacion))==-1)
				{
		        	printf("\nTORNEO: Error al recibir paquete de datos del Cliente que se conectó fuera de tiempo.\n");
		        	kill(getpid(),SIGTERM);
		        }
		        paquete.numero=-1;
				if(enviarDatos(&aux.clifd,&paquete,sizeof(t_identificacion))==-1)
        			printf("\nTORNEO: Error al enviar paquete de datos a Cliente que se conectó fuera de tiempo.\n");
        		terminarConexion(&aux.clifd);
			}
  	}
}

/////////////////////////////////////////////////////////////////////////

void *verificaMonitor(void *param) //thread que verifica que no se haya terminado el modulo del proceso monitor
{
  
	mtxmonitor=obtenerMutex("/mtxmonitor"); //obtiene el mutex que permite la ejecucion secuencial de los threads que esten verificando
                                  //la existencia del proceso monitor
	while(1)
  	{
	    pedirSemaforo(mtxmonitor);
	    system("./verifica_monitor.sh"); //ejecuta script que verifica la existencia del monitor y lo relanza al no encontrarlo
	    devolverSemaforo(mtxmonitor);
	    sleep(1);
  	}
}

////////////////////////////////////////////////////////////////////////

int main() //main. Programa.
{
	XInitThreads();//
	int valor_p=0;
	pid_t monitor=0;
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction=handler; //acciones correspondientes al manejo de la 
	sigfillset(&act.sa_mask); //estructura sigaction. 
	act.sa_flags=SA_SIGINFO; //indicacion de uso de la funcion handler
	   				 //SIGINT y SIGTERM. Demas senales
	sigaction(SIGTERM,&act,NULL); //se trabajan con su respuesta por defecto.
	sigaction(SIGINT,&act,NULL);
	//sigaction(SIGUSR1,&act,NULL); //SIGUSR1 para la finalizacion correcta de threads
	signal(SIGUSR1,SIG_IGN);
	signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
	signal(SIGUSR2,SIG_IGN); //se ignora SIGUSR2
	for(int i=0;i<5;i++)
		hilo[i]=0;
	mtxTorneoPartida=obtenerMutex("/mtxTorneoPartida");

	CLAVE=ftok(".",'M');
	if(CLAVE==-1)
	{
		printf("\nError al intentar crear clave de memoria compartida. Fin\n");
		exit(0);
	}
	if((shmid=shmget(CLAVE, sizeof(t_paquete_serv), 0660 | IPC_CREAT))==-1) //IPC_EXCL
	{
		printf("\nError al intentar obtener ID de memoria compartida. Fin\n");
		exit(1);
	}

	if(cargarConfiguracion("configTorneo.cfg")<0)
	{
			printf("Error al cargar la configuración del torneo\n");
			exit(1);
	}
	char auxDif[12];
	switch(DIFICULTAD)
	{
		case 0: strcpy(auxDif,"Brasil");break;
		case 1: strcpy(auxDif,"Normal");break;
		case 2: strcpy(auxDif,"Extreme");break;
	}
	tiempo=TIEMPO_INSCRIPCION;
  	if((fork())==0) //creacion del proceso demonio
  	{
  		demonio=getpid();
		iniciarSDL();
		act.sa_sigaction=handler; //acciones correspondientes al manejo de la 
		sigfillset(&act.sa_mask); //estructura sigaction. 
		act.sa_flags=SA_SIGINFO; //indicacion de uso de la funcion handler
	   				 //SIGINT y SIGTERM. Demas senales
		sigaction(SIGTERM,&act,NULL); //se trabajan con su respuesta por defecto.
		sigaction(SIGINT,&act,NULL);
		// sigaction(SIGUSR1,&act,NULL); //SIGUSR1 para la finalizacion correcta de threads
		signal(SIGUSR1,SIG_IGN);
		signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
		signal(SIGUSR2,SIG_IGN); //se ignora SIGUSR2

	    sleep(1); //suspension de proceso(para matar a su padre y asi quedar 

	    //MEMORIA COMPARTIDA
		if((shmid=shmget(CLAVE, sizeof(t_paquete_serv), 0660))==-1)
		{
			printf("\nError al intentar obtener ID de memoria compartida. Fin\n");
			exit(1);
		}
		if((buffer=(t_paquete_serv*)shmat(shmid,(char*)0,0))==NULL)
		{
			printf("\nError en attach de segmento de memoria compartida. Fin. server Partida \n");
			exit(1);
		}
	    printf("\n\nINICIANDO SERVER TORNEO [%d]\n",getpid());
	    printf("\nPUERTO: %d\nINSCRIPCIÓN: %d segundos\nDIFICULTAD: %s\nRESCATES: %d\n",PUERTO,TIEMPO_INSCRIPCION,auxDif,RESCATES);
	    //
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
        fprintf(fpid,"%s\n", "servidor");
        pclose(fp);
   		fclose(fpid);

   		// fclose(fpid);
   		// sprintf(comando,"chmod a+r %s",DIR_PID);
   		// system(comando);
    	//Monitor
    	printf("LANZANDO PROCESO MONITOR\n");
		valor_p = pthread_create(&hilo[cantThreads],NULL,verificaMonitor,NULL); //lanza thread de verificacion de proceso monitor
		if(valor_p)
		{
			printf("Error en la creacion de hilos(threads) de verificar Monitor. Error devuelto: %d. Fin del programa\n",valor_p);
			kill(getpid(), SIGTERM); 
		}
		cantThreads++;
		//
		valor_p = pthread_create(&hilo[cantThreads],NULL,obtener_Y_AceptarConexiones,NULL); //thread que crea y acepta conexiones de clientes
		if(valor_p)
		{
			printf("Error en la creacion de hilos(threads) de manejo de mensajes. Error devuelto: %d. Fin del programa\n",valor_p);
			kill(getpid(), SIGTERM);  
		}
		cantThreads++;
     	printf("\n");
     	//ESPERA DE CONEXIONES
	    while(tiempo!=0)
	    {
		    printf("Tiempo restante de espera de conexiones: %02d seg", tiempo);
		    printf("\r");
			cargarEsperaEnPantalla(tiempo);
			fflush(stdout);
		    sleep(1);
		    tiempo--; 
	    }

	    //pthread_cancel(hilo[1]);    //Se acaba el tiempo de escucha, se cierra el thread. 
		servidoresPartida=clientesConectados/2;
		//printf("\nCantidad de servidores de Partida a iniciar: %d\n",servidoresPartida);
		cargarCantidadDeConectadosEnPantalla(clientesConectados);
		//Informo al cliente sin pareja, que quedó fuera del torneo
		if((clientesConectados%2) !=0)
		{	    
      		t_identificacion paquete;
      		paquete.numero=-1;
      		clientesConectados--;
			if(enviarDatos(&cliente[clientesConectados].clifd,&paquete,sizeof(t_identificacion))==-1)
        		printf("\nError al enviar paquete de datos a Cliente que quedo afuera. server de t PID=%d\n", getpid());
     	}
     	//
     	if(clientesConectados==0)
     	{
     		eliminarPID();
     		printf("No hay clientes suficientes para lanzar una partida. Finalizando torneo.\n");
     		kill(demonio, SIGTERM);
     	}
     	printf("ASIGNANDO PARTIDAS\n");
   		int posaux=clientesConectados;   // Ìndice (Desde el final del array)
   		//printf("\nValor posaux: %d\n",posaux);
   		for(int h=0;h<servidoresPartida;h++)
   		{
   			char c1[5], c2[5], par3[5], nroCli1[5], nroCli2[5], resc[5],dif[5];
			sprintf(c1,"%d",cliente[h].clifd);
			sprintf(c2,"%d",cliente[posaux-1].clifd);
			sprintf(par3,"%d",h);
			sprintf(nroCli1,"%d",h);
			sprintf(nroCli2,"%d",posaux-1);
			sprintf(resc,"%d",RESCATES);
			sprintf(dif,"%d",DIFICULTAD);
			//Inicializamos los datos de la partida
			strcpy(datosPartida[h].nombre1,datosCliente[h].nombre);
			strcpy(datosPartida[h].nombre2,datosCliente[posaux-1].nombre);
			datosPartida[h].rescates[0]=datosPartida[h].rescates[1]=0;
			datosPartida[h].vidasPerdidas[0]=datosPartida[h].vidasPerdidas[1]=0;
			datosPartida[h].clienteGanador=-1;
		  	printf("Partida %d: %s vs %s\n",h,datosPartida[h].nombre1,datosPartida[h].nombre2);
		    if((serverP[h]=vfork())==0)
		    {
		    	//serverP[h]=getpid();
		    	//printf("Serverp[%d]: %d\n",h,getpid());
			    if(execlp("./serverPartida","serverPartida",c1,c2,par3,nroCli1,nroCli2,resc,dif,(char*)NULL)<0)
		        {
		        	printf("\nError en la creacion del servidor partida. execlp() .Fin\n");
		        	kill(demonio, SIGTERM);
		        }
				_exit(0);
		    }
    		if(serverP[h]<0)
    		{
       			printf("\nError al crear proceso Servidor Partida. Fin\n");
       			kill(getpid(), SIGTERM); 
     		}
     		posaux--;
    	}
    	sleep(3);
  		printf("\n\nSERVER TORNEO: Enviando confirmación a los jugadores aceptados\n");
    	//Informo a los demas clientes que fueron aceptados en el torneo
    	for(int a=0;a<clientesConectados;a++)
		{	    
      		t_identificacion paquete;
      		paquete.numero=2;
      		printf("Confirmando: %s\n",datosCliente[a].nombre);
			if(enviarDatos(&datosCliente[a].clifd,&paquete,sizeof(t_identificacion))==-1)
        		printf("\nError al enviar paquete de datos al cliente %d\n",a);
     	}

     	//Inicio del thread de manejo de pantalla
		valor_p = pthread_create(&hilo[cantThreads],NULL,SDLtorneo,NULL); //thread que crea y acepta conexiones de clientes
		if(valor_p)
		{
			printf("Error en la creacion del hilo(thread) de comunicación con los ervidores de partida. Error devuelto: %d. Fin del programa\n",valor_p);
			kill(getpid(), SIGTERM); 
		}
		cantThreads++;
    	//Inicio del thread que recibe el estado de las partidas
		valor_p = pthread_create(&hilo[cantThreads],NULL,verificarEstadoDePartidas,NULL); //thread que crea y acepta conexiones de clientes
		if(valor_p)
		{
			printf("Error en la creacion del hilo(thread) de comunicación con los ervidores de partida. Error devuelto: %d. Fin del programa\n",valor_p);
			kill(getpid(), SIGTERM); 
		}
		cantThreads++;
		//Inicio del thread de reasignación de partidas
		valor_p = pthread_create(&hilo[cantThreads],NULL,lanzarProximaRonda,NULL); //thread que crea y acepta conexiones de clientes
		if(valor_p)
		{
			printf("Error en la creacion de hilos(threads) de manejo de mensajes. Error devuelto: %d. Fin del programa\n",valor_p);
			kill(getpid(), SIGTERM);  
		}
		cantThreads++;

	  	while(1) 
	    {	
	    	pause();   
	    } 
		printf("\nSoy el server de Torneo. No deberia haber llegado aca. Me autoenvio SIGTERM");
		kill(getpid(), SIGTERM); 
	}
  	else
  	{
   		if(demonio==-1)
   		{
     		printf("Error al crear proceso Server Partida(Demonio). Fin\n");
     		exit(0); 
   		}
  	}
 	exit(0); //muerte del proceso padre principal. Fuerza al hijo a quedar como demonio
}

/*********************************************************************************************/

int cargarConfiguracion(const char*path)
	{
		FILE* pf;
		char buffer[100];
		char valor[100];
		char*aux;
		pf=fopen(path, "rt");
		if(!pf)
		{
			printf("Error: problema al leer el archivo de configuración\n");
			return -1;
		}
		bzero(buffer, sizeof(buffer));
		//PUERTO
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux++;
		sscanf(aux,"%d",&PUERTO);
		if(PUERTO<=0)
		{
			printf("Puerto NO válido.\n");
			return -1;
		}
		//TIEMPO DE INSCRIPCIÓN
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux++;
		sscanf(aux,"%d",&TIEMPO_INSCRIPCION);
		if(TIEMPO_INSCRIPCION<=0)
		{
			printf("Tiempo de inscripción NO válido.\n");
			return -1;
		}
		//DIFICULTAD
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		if(strcmp(valor,"Brasil")==0)
			DIFICULTAD=0;
		else
			if(strcmp(valor,"Normal")==0)
				DIFICULTAD=1;
			else
				if(strcmp(valor,"Extreme")==0)
					DIFICULTAD=2;
				else
				{
					DIFICULTAD=1; 	//VALOR POR DEFECTO (Normal)
					printf("Nivel de dificultad NO válido: se utilizará \"Normal\"\n");
				}
		//CANTIDAD DE RESCATES
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux++;
		sscanf(aux,"%d",&RESCATES);
		if(RESCATES<=0)
		{
			printf("Cantidad de rescates NO válida: debe ser mayor a cero\n");

		}
		fclose(pf);
		return 0;
	}

/////////////////////////////////////////////////// S D L /////////////////////////////////////////////////////////////////////////

void iniciarSDL()
{
	SDL_Rect pos=(SDL_Rect){0,0,0,0};
	SDL_Event event;
	atexit(SDL_Quit);
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
		printf("No se ha podido iniciar SDL: %s\n", SDL_GetError());
		exit(1);
	}
	if(TTF_Init()==-1) 
  {  //inicializa el sistema de fuentes.
		fprintf(stderr, "Error al inicializar SDL_TTF: %s\n",TTF_GetError());
		exit(1);
	}
	pantalla=SDL_SetVideoMode(ANCHO, ALTO, 32, SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
	if(pantalla==NULL)
  {
		printf("No se ha podido establecer el modo de vídeo: %s\n", SDL_GetError());
    	exit(1);
	}
	SDL_WM_SetCaption("Servidor Donkey-Kong",NULL);
	fuente=cargarFuente(FONT,13);
	fuenteVictoria=cargarFuente(FONT,20);
}

void cargarEsperaEnPantalla(int tiempo)
{
  char informe_tiempo[100];
  char segundo[4];
  borrarSectorDePantalla(15,15,ANCHO,20);
  strcpy(informe_tiempo , "Tiempo de espera restante: " );  
  sprintf(segundo,"%d",tiempo);
  strcat(informe_tiempo,segundo);   
  strcat(informe_tiempo," segundos.");   
  create_label(15, 15, informe_tiempo,BLANCO,fuente);  
  if(tiempo==0)
  {
  	borrarSectorDePantalla(15,15,ANCHO,20); 
  	create_label(15, 15, "Tiempo de espera finalizado, no se recibirán más clientes.", BLANCO,fuente);   	
  }
}

void borrarSectorDePantalla(int x, int y, int w, int h)
{
  SDL_Rect borrar;
  borrar.x=x;
  borrar.y=y;
  borrar.w=w;
  borrar.h=h;
  SDL_FillRect(pantalla, &borrar, SDL_MapRGB(pantalla->format, 0, 0, 0));
}

void cargarCantidadDeConectadosEnPantalla(int cant)
{
  SDL_FillRect(pantalla,NULL,SDL_MapRGB(pantalla->format, 0, 0, 0));
  char informe_tiempo[100];
  char cantidad[4];
  strcpy(informe_tiempo , "Cantidad de clientes conectados: ");  
  sprintf(cantidad,"%d",cant);
  strcat(informe_tiempo,cantidad);   
  create_label(15, 15,informe_tiempo,BLANCO,fuente);
  strcpy(informe_tiempo , "Cantidad de partidas: ");  
  sprintf(cantidad,"%d",cant/2);
  strcat(informe_tiempo,cantidad);   
  create_label(15, 30,informe_tiempo,BLANCO,fuente);
}

void create_label(int x, int y, const char *file, SDL_Color color,TTF_Font *font)
{
    SDL_Surface *img_label;
    SDL_Rect pos_label;
    SDL_Color paleta_color;
    //posicion etiqueta
    pos_label = (SDL_Rect) {x, y, 0, 0};
 	img_label = TTF_RenderText_Solid(font, file, color);
    //volcamos superficies en buffer
    SDL_BlitSurface(img_label, NULL, pantalla, &pos_label);
    //pintamos imagen
    SDL_Flip(pantalla);
    //liberamos superficies
    SDL_FreeSurface(img_label);
}

void informarEnPantalla()
{
	char cad[200];
	bzero(cad,200);
	sprintf(cad,"PARTIDAS EN CURSO");
	borrarSectorDePantalla(15,15,350,15);
	create_label(15,15,cad,CELESTE,fuente);
	for(int x=0;x<servidoresPartida;x++)
	{
		bzero(cad,200);
		if(datosPartida[x].rescates[0]>=RESCATES)
			sprintf(cad,"Partida %d:  Ganador: %s\0",x,datosPartida[x].nombre1);
		else
			if(datosPartida[x].rescates[1]>=RESCATES)
				sprintf(cad,"Partida %d:  Ganador: %s\0",x,datosPartida[x].nombre2);
			else
				sprintf(cad,"Partida %d: %s vs %s\0",x,datosPartida[x].nombre1,datosPartida[x].nombre2);
		borrarSectorDePantalla(15,30+45*x,350,15);
		create_label(15,30+45*x,cad,BLANCO,fuente);
		bzero(cad,200);
		sprintf(cad,"%s:  Rescates: %d   Vidas perdidas: %d\0",datosPartida[x].nombre1,datosPartida[x].rescates[0],datosPartida[x].vidasPerdidas[0]);
		borrarSectorDePantalla(45,45+45*x,350,15);
		create_label(45,45+45*x,cad,BLANCO,fuente);
		bzero(cad,200);
		sprintf(cad,"%s:  Rescates: %d   Vidas perdidas: %d\0",datosPartida[x].nombre2,datosPartida[x].rescates[1],datosPartida[x].vidasPerdidas[1]);
		borrarSectorDePantalla(45,60+45*x,350,15);
		create_label(45,60+45*x,cad,BLANCO,fuente);
	}
	bzero(cad,200);
	sprintf(cad,"PARTIDAS FINALIZADAS");
	borrarSectorDePantalla(400,15,180,15);
	create_label(400,15,cad,CELESTE,fuente);
	bzero(cad,200);
	sprintf(cad,"RESULTADO");
	borrarSectorDePantalla(620,15,ANCHO,15);
	create_label(620,15,cad,CELESTE,fuente);
	for(int i=0;i<cantPartidasFinalizadas;i++)
	{
		bzero(cad,200);
		sprintf(cad,"%s vs %s",partidasFinalizadas[i].nombre1,partidasFinalizadas[i].nombre2);
		borrarSectorDePantalla(400,30+15*i,200,15);
		create_label(400,30+15*i,cad,BLANCO,fuente);
		bzero(cad,200);
		sprintf(cad,"Ganador: %s",partidasFinalizadas[i].ganador);
		borrarSectorDePantalla(620,30+15*i,ANCHO,15);
		create_label(620,30+15*i,cad,VERDE,fuente);
	}
	if(HAY_CAMPEON==1)
	{
		bzero(cad,200);
		sprintf(cad,"Ganador del torneo:");
		borrarSectorDePantalla(200,300,200,20);
		create_label(200,300,cad,BLANCO,fuenteVictoria);
		borrarSectorDePantalla(400,300,ANCHO,20);
		create_label(400,300,campeon,VERDE,fuenteVictoria);
	}
}

/*********************************************************************************************/

int buscarPidServidor(int pid)
{
	for(int i=0;i<servidoresPartida;i++)
		if(serverP[i]==pid)
			return i;
	return -1;
}

/*********************************************************************************************/
void manejarEvento(SDL_Event evento)
	{
		Uint8 *keys;
		switch(evento.type)
		{
			case SDL_QUIT:
							FINALIZAR_TORNEO=1;
							break;
			// case SDL_KEYDOWN:
			// 				keys = SDL_GetKeyState(NULL);
			// 				TECLA PARA SALIR DEL JUEGO
			// 				if(keys[SDLK_q].salir])
			// 				{
			// 					FINALIZAR_TORNEO=1;
			// 					break;
			// 				}
		}
	}

/*********************************************************************************************/
//Threads
/*********************************************************************************************/
	void* SDLtorneo(void*param)
	{
		SDL_Event evento;
		while(FINALIZAR_TORNEO==0)
		{
			informarEnPantalla();
			if(SDL_PollEvent(&evento))		//Verificamos si ocurre un evento en el juego
			{
				manejarEvento(evento);
			}
			SDL_Delay(50);
		}
		//eliminarPID();
		kill(demonio, SIGTERM);
	}

	/*********************************************************************************************/

	void* verificarEstadoDePartidas(void*param)
	{
		while(1)
		{
				pedirSemaforo(mtxTorneoPartida);
				switch(buffer->evento1)
				{
					case 0:
							break;
					case 1:
							datosPartida[buffer->id_serverPartida].vidasPerdidas[buffer->nroJugador1]++;
							printf("PARTIDA %d: EL jugador %s fue golpeado por una llama!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre1);
							break;
					case 2:
							datosPartida[buffer->id_serverPartida].vidasPerdidas[buffer->nroJugador1]++;
							printf("PARTIDA %d: %s fue golpeado por un barril!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre1);
							break;
					case 3:
							datosPartida[buffer->id_serverPartida].rescates[buffer->nroJugador1]++;
							printf("PARTIDA %d: %s rescató a la princesa!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre1);
							break;
					case 4:
							printf("PARTIDA %d: %s obtuvo un martillo!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre1);
							break;
					case 5:
							datosPartida[buffer->id_serverPartida].rescates[buffer->nroJugador1]=RESCATES;
							datosPartida[buffer->id_serverPartida].clienteGanador=buffer->socketGanador;
							CANT_GANADORES++;
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].nombre1,datosPartida[buffer->id_serverPartida].nombre1);
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].nombre2,datosPartida[buffer->id_serverPartida].nombre2);
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].ganador,datosPartida[buffer->id_serverPartida].nombre1);
							cantPartidasFinalizadas++;
							printf("PARTIDA %d: %s ganó la partida!\tSocket: %d\n",buffer->id_serverPartida, datosCliente[buffer->socketGanador].nombre,buffer->socketGanador);
							buffer->evento1=0;
							break;
					default:
							break;
				}
				buffer->evento1=0;
				switch(buffer->evento2)
				{
					case 0:
							break;
					case 1:
							datosPartida[buffer->id_serverPartida].vidasPerdidas[buffer->nroJugador2]++;
							printf("PARTIDA %d: %s fue golpeado por una llama!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre2);
							break;
					case 2:
							datosPartida[buffer->id_serverPartida].vidasPerdidas[buffer->nroJugador2]++;
							printf("PARTIDA %d: %s fue golpeado por un barril!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre2);
							break;
					case 3:
							datosPartida[buffer->id_serverPartida].rescates[buffer->nroJugador2]++;
							printf("PARTIDA %d: %s rescató a la princesa!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre2);
							break;
					case 4:
							printf("PARTIDA %d: %s obtuvo un martillo!\n",buffer->id_serverPartida, datosPartida[buffer->id_serverPartida].nombre2);
							break;
					case 5:
							datosPartida[buffer->id_serverPartida].rescates[buffer->nroJugador2]=RESCATES;
							datosPartida[buffer->id_serverPartida].clienteGanador=buffer->socketGanador;
							CANT_GANADORES++;
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].nombre1,datosPartida[buffer->id_serverPartida].nombre1);
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].nombre2,datosPartida[buffer->id_serverPartida].nombre2);
							strcpy(partidasFinalizadas[cantPartidasFinalizadas].ganador,datosPartida[buffer->id_serverPartida].nombre2);
							cantPartidasFinalizadas++;
							printf("PARTIDA %d: %s ganó la partida!\tSocket: %d\n",buffer->id_serverPartida, datosCliente[buffer->socketGanador].nombre,buffer->socketGanador);
							buffer->evento2=0;
							break;
					default:
							break;
				}
				buffer->evento2=0;
				devolverSemaforo(mtxTorneoPartida);
				SDL_Delay(1);
		}
	}

	/*********************************************************************************************/

	void* lanzarProximaRonda(void*param)
	{
		while(1)
		{
			if(ASIGNANDO_PARTIDA==0)
			{
				if(CANT_GANADORES==servidoresPartida&&servidoresPartida!=0)
				{
					asignarPartidas();
				}
			}
			SDL_Delay(1);
		}
	}

	/*****************************************************************************************************/

	void asignarPartidas()
	{
		ASIGNANDO_PARTIDA=1;
		 // for(int x=0;x<servidoresPartida;x++)
		 // 	kill(serverP[x], SIGTERM);
				for(int a=0;a<6;a++)
		   		{
		   			printf("Iniciando nueva ronda en %d",6-1-a);
		   			printf("\r");
		   			sleep(1);
		   			fflush(stdout);
		   		}
		   		printf("\n");
				if(CANT_GANADORES==1)
				{
					//eliminarPID();
					int ganador=datosPartida[0].clienteGanador;
					strcpy(campeon,datosCliente[ganador].nombre);
					printf("%s ES EL GANADOR DEL TORNEO!\n",datosCliente[ganador].nombre);
					t_identificacion paquete;
		      		paquete.numero=3;
					if(enviarDatos(&datosCliente[ganador].clifd,&paquete,sizeof(t_identificacion))==-1)
		        		printf("\nError al enviar paquete de datos al cliente %d\n",0);
		        	ASIGNANDO_PARTIDA=1;
		      		HAY_CAMPEON=1;
		        	//printf("FINALIZANDO TORNEO\n");
		        	//sleep(10);
		        	//exit(0);
		        	//sleep(10);
		        	//kill(demonio, SIGTERM); 				  //MATA AL TORNEO?
		        	//kill(getpid(),SIGTERM);                 //MATA AL MONITOR?
		        	//pthread_cancel(hilo[1]);
		        	//raise(SIGTERM);
				}
				else
				{
					printf("LANZANDO PROXIMA RONDA\n");
					if((CANT_GANADORES%2)!=0 && CANT_GANADORES>1)
					{	    
			      		t_identificacion paquete;
			      		paquete.numero=-1;
			      		CANT_GANADORES--;
						if(enviarDatos(&datosPartida[servidoresPartida].clienteGanador,&paquete,sizeof(t_identificacion))==-1)
						{
							printf("\nError al enviar paquete de datos a Cliente que quedo afuera. server de t PID: %d\n", getpid());
							//kill(getpid(),SIGTERM);
							kill(demonio, SIGTERM);
							//raise(SIGTERM);
						}
			     	}
			     	servidoresPartida=CANT_GANADORES/2;
			   		int posaux=CANT_GANADORES;   // Ìndice (Desde el final del array)
			   		for(int h=0;h<servidoresPartida;h++)
			   		{
			   			int ganador1=datosPartida[h].clienteGanador;
			   			int ganador2=datosPartida[posaux-1].clienteGanador;
			   			datosPartida[h].clienteGanador=-1;
			   			datosPartida[posaux-1].clienteGanador=-1;
			   			char c1[5], c2[5], par3[5], nroCli1[5],nroCli2[5], resc[5],dif[5];
						sprintf(c1,"%d",datosCliente[ganador1].clifd);
						sprintf(c2,"%d",datosCliente[ganador2].clifd);
						sprintf(par3,"%d",h);
						sprintf(nroCli1,"%d",ganador1);
						sprintf(nroCli2,"%d",ganador2);
						sprintf(resc,"%d",RESCATES);
						sprintf(dif,"%d",DIFICULTAD);
						//Inicializamos los datos de la partida
						strcpy(datosPartida[h].nombre1,datosCliente[ganador1].nombre);
						strcpy(datosPartida[h].nombre2,datosCliente[ganador2].nombre);
						datosPartida[h].rescates[0]=datosPartida[h].rescates[1]=0;
						datosPartida[h].vidasPerdidas[0]=datosPartida[h].vidasPerdidas[1]=0;
						datosPartida[h].clifd[0]=datosCliente[ganador1].clifd;
						datosPartida[h].clifd[1]=datosCliente[ganador2].clifd;
						//
					  	printf("Partida %d: %s vs %s\n",h,datosCliente[ganador1].nombre,datosCliente[ganador2].nombre);
					  	printf("NroClientes %d: %d vs %d\n",h,ganador1,ganador2);
					  	printf("Sockets %d: %s vs %s\n",h,c1,c2);
					    if((serverP[h]=vfork())==0)
					    {
						    if(execlp("./serverPartida","serverPartida",c1,c2,par3,nroCli1,nroCli2,resc,dif,(char*)NULL)<0)
					        {
					        	printf("\nError en la creacion del servidor partida. execlp() .Fin\n");
					        	kill(demonio, SIGTERM);
					        }
							_exit(0);
					    }
			    		if(serverP[h]<0)
			    		{
			       			printf("\nError al crear proceso Servidor Partida. Fin\n");
			       			kill(demonio, SIGTERM);
			     		}
			     		posaux--;
			     		//Informo a los clientes que fueron aceptados en el torneo
				      	t_identificacion paquete;
				      	paquete.numero=2;
						if(enviarDatos(&datosCliente[ganador1].clifd,&paquete,sizeof(t_identificacion))==-1)
				        	printf("\nError al enviar paquete de datos al cliente %d\n",ganador1);
				        if(enviarDatos(&datosCliente[ganador2].clifd,&paquete,sizeof(t_identificacion))==-1)
				        	printf("\nError al enviar paquete de datos al cliente %d\n",ganador2);
			    	}
			    	printf("SERVIDOR DE TORNEO: Nueva ronda lanzada\n");
				}
		     	CANT_GANADORES=0;
		ASIGNANDO_PARTIDA=0;
	}