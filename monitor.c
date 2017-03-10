/* Sistemas Operativos - Trabajo Practico 4 / Monitor
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
	#include <signal.h>
	#include <stdlib.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/msg.h>
	#include <string.h>
	#include <sys/msg.h>
	#include <pthread.h>
	#include <fcntl.h>
	#include <dirent.h>
	#include <semaphore.h>
	#include <sys/inotify.h>
	#include <SDL/SDL.h>
	#include "./socketsLib/socketsLib.h"
	#include "./semaforosLib/semaforos.h"

//DEFINES
	#define LLAVE (key_t)234 
	#define EVENT_SIZE (sizeof(struct inotify_event))
	#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 100))

//STRUCTS
	typedef struct
	{
	  	int activo;
	  	pid_t pid;
	  	char nombre[20], ip[20];
	}t_proceso;

//VARIABLES GLOBALES
	pid_t monitor; //pid del proceso monitor
	int fd,wd,cant_proc=0;
	t_proceso proceso[20];
	FILE *fpid=NULL;
	char ipmon[20];
	//pthread_mutex_t mtxlectura=PTHREAD_MUTEX_INITIALIZER;
	sem_t *mtxPIDS=NULL;

	pthread_t hilo[2]; //pids de los threads
	//t_proceso proceso[50];
	//pthread_mutex_t imprimemtx=PTHREAD_MUTEX_INITIALIZER; //semaforos mutex a utilizar en los threads

//PROTOTIPOS DE FUNCIONES
	void leerArchivosPID();
	void* actualizarArchivosPID(void*param);

/**************************************************************************************************/

void handler(int iNumSen,siginfo_t *info,void *ni) //funcion declarada para el
{                //manejo de las senales recibidas por el proceso monitor.
  	int u=0,flg=0;
	FILE *faux=NULL;
	//int ERRStream = open("/dev/null", O_WRONLY);
 	switch(iNumSen)
  	{
	  	case SIGUSR1:
	  				//dup2(STDERR_FILENO,ERRStream);
					break;

  		case SIGINT: //respuesta a la senal SIGINT
  					//dup2(STDERR_FILENO,ERRStream);
			  		signal(SIGINT,SIG_IGN);
			  		printf("\nSe canceló el programa Monitor (SIGINT).\n");
			  		raise(SIGTERM); //se autoenvia la senal SIGTERM para que finalice
			  		break;               //correctamente el proceso
 
  		case SIGTERM: //caso de finalizacion del programa 
					//pthread_mutex_destroy(&mtxlectura);
  					//dup2(STDERR_FILENO,ERRStream);
					eliminarSemaforo(mtxPIDS,"/mtxPIDS");
					if(info->si_pid==getpid())
						printf("MONITOR: Fin del programa 1\n");
					else
						printf("Se finalizo el programa monitor (SIGTERM).\n");  
					exit(0); 
					break;  //se finaliza el proc. monitor.
   	}
}

//////////////////////////////FUNCIONES/////////////////////////////////////

void leerArchivosPID()
{
 // pthread_mutex_lock(&mtxlectura);
	pedirSemaforo(mtxPIDS);
  	DIR *directorio=NULL;
  	char patharch[20], nom_dir[10], buffer[20], *paux=NULL;
  	struct dirent *entrada;
  	FILE *fpid=NULL;
  	cant_proc=0;
  	//int cant_leidos=0;
	//printf("Entre a LEERARCHIVOS PID\n");
  	strcpy(nom_dir,"./.PID/");
   	directorio=opendir(nom_dir);
  	if(directorio==NULL)
  	{ //validacion de acceso al directorio deseado(enviado por parametro)
  		printf("Error al intentar abrir el directorio indicado. Fin del programa\n");
  		//exit(1);
  	}
  
 	while((entrada=readdir(directorio))!=NULL)
  	{
   		if( (strcmp(entrada->d_name, ".")!=0) && (strcmp(entrada->d_name, "..")!=0))
		{
		 	//strcpy(enviad.archivo, nom_dir); //lee los archivos del directorio que se encuentran al
			fpid=fopen(entrada->d_name,"r"); //momento de iniciar el proceso monitor. 
		 	//fpid=fopen("./PID/serverTorneo.txt","r");
	  		sprintf(patharch,"%s%s",nom_dir,entrada->d_name);
	  		fpid=fopen(patharch,"r"); //momento de iniciar el proceso monitor. 
		 	//fpid=fopen("./PID/serverTorneo.txt","r");
	  		//printf("HICE FOPEN\n");
			if(fpid)
			{
		  		fgets(buffer,20,fpid);
		  		paux=strrchr(buffer,'\n');
		  		if(paux)
					*paux='\0';
		  		if(strcmp(buffer,ipmon)==0)
		  		{
		  			strcpy(proceso[cant_proc].ip, buffer);
		   			proceso[cant_proc].pid=atoi(entrada->d_name);
		   			proceso[cant_proc].activo=1;
		   			fgets(buffer,20,fpid);
			  		paux=strrchr(buffer,'\n');
			  		if(paux)
						*paux='\0';
		   			strcpy(proceso[cant_proc].nombre,buffer);
		   			//strcpy(proceso[cant_proc].ip,buffer);
					//printf("IP: %s\tPID: %d\tNombre: %s\n", proceso[cant_proc].ip, proceso[cant_proc].pid,proceso[cant_proc].nombre);
					cant_proc++;
		  		}
				fclose(fpid);
			}
			fpid=NULL;
		 	//cant_leidos++;
		}
  	}
  	closedir(directorio); //cierra el directorio
	if(cant_proc==0)
	{
		sleep(2);
		system("./eliminar_todo_ipcs.sh");
	  	kill(monitor,SIGTERM);
	}
   //pthread_mutex_unlock(&mtxlectura);
   devolverSemaforo(mtxPIDS);
}

/********************************************************************************************************/

void *actualizarArchivosPID(void *param)
{
	while(1)
	{
		pedirSemaforo(mtxPIDS);
	  	DIR *directorio=NULL;
	  	char patharch[20], nom_dir[10], buffer[20], *paux=NULL;
	  	struct dirent *entrada;
	  	FILE *fpid=NULL;
	  	cant_proc=0;
	  	//int cant_leidos=0;
		//printf("Entre a LEERARCHIVOS PID\n");
	  	strcpy(nom_dir,"./.PID/");
	   	directorio=opendir(nom_dir);
	  	if(directorio==NULL)
	  	{ //validacion de acceso al directorio deseado(enviado por parametro)
	  		printf("Error al intentar abrir el directorio indicado. Fin del programa\n");
	  		//exit(1);
	  	}
	  	//printf("/***********************************************************************/\n");
	 	while((entrada=readdir(directorio))!=NULL)
	  	{
	   		if( (strcmp(entrada->d_name, ".")!=0) && (strcmp(entrada->d_name, "..")!=0))
			{
			 	//strcpy(enviad.archivo, nom_dir); //lee los archivos del directorio que se encuentran al
				fpid=fopen(entrada->d_name,"r"); //momento de iniciar el proceso monitor. 
			 	//fpid=fopen("./PID/serverTorneo.txt","r");
		  		sprintf(patharch,"%s%s",nom_dir,entrada->d_name);
		  		fpid=fopen(patharch,"r"); //momento de iniciar el proceso monitor. 
			 	//fpid=fopen("./PID/serverTorneo.txt","r");
		  		//printf("HICE FOPEN\n");
				if(fpid)
				{
			  		fgets(buffer,20,fpid);
			  		paux=strrchr(buffer,'\n');
			  		if(paux)
						*paux='\0';
			  		if(strcmp(buffer,ipmon)==0)
			  		{
			  			strcpy(proceso[cant_proc].ip, buffer);
			   			proceso[cant_proc].pid=atoi(entrada->d_name);
			   			proceso[cant_proc].activo=1;
			   			fgets(buffer,20,fpid);
				  		paux=strrchr(buffer,'\n');
				  		if(paux)
							*paux='\0';
			   			strcpy(proceso[cant_proc].nombre,buffer);
			   			//strcpy(proceso[cant_proc].ip,buffer);
						//printf("IP: %s\tPID: %d\tNombre: %s\n", proceso[cant_proc].ip, proceso[cant_proc].pid,proceso[cant_proc].nombre);
						cant_proc++;
			  		}
					fclose(fpid);
				}
				fpid=NULL;
			 	//cant_leidos++;
			}
	  	}
	  	closedir(directorio); //cierra el directorio
		if(cant_proc==0)
		{
			sleep(2);
			devolverSemaforo(mtxPIDS);
			system("./eliminar_todo_ipcs.sh");
		  	kill(monitor,SIGTERM);
		}
	   //pthread_mutex_unlock(&mtxlectura);
	   devolverSemaforo(mtxPIDS);
	   SDL_Delay(200);
	}
}

/////////////THREADS/////////////////////////////

void *monitorearProcesos(void *param) //thread que monitorea procesos
{
  	int x=1,h=0;
  	char comando[50];
	while(x)
	{
		//pthread_mutex_lock(&mtxlectura);
		//pedirSemaforo(mtxPIDS);
		for(h=0;h<cant_proc;h++)
		 {
		 	if(cant_proc==0)
		 		x=1;
			if(strcmp(proceso[h].ip,ipmon)==0 && proceso[h].activo==1)
			{
				if(kill(proceso[h].pid,SIGUSR2)!=0) //envia una senal al pid indicado. Esta senal es ignorada en los procesos
				{
					if(strcmp(proceso[h].nombre,"servidor")==0 || strcmp(proceso[h].nombre,"partida")==0) //SI CAE ALGÚN SERVIDOR
					{
						proceso[h].activo=0;
						printf("\nMONITOR: El modulo %s [%d] se ha perdido. Cancelando el torneo y liberando recursos...\n", proceso[h].nombre,proceso[h].pid);
						//sprintf(comando,"rm ./.PID/%d",proceso[h].pid);
						system("./eliminar_todo_ipcs.sh 2> /dev/null");
						x=0;
					}
					else
					{
						proceso[h].activo=0;
						printf("\nMONITOR: El modulo %s [%d] se ha perdido. Eliminando recursos del proceso...\n", proceso[h].nombre,proceso[h].pid);
						sprintf(comando,"rm ./.PID/%d 2> /dev/null",proceso[h].pid);
						system(comando);
					}
				}
		   }
		}
		//pthread_mutex_unlock(&mtxlectura);
		//devolverSemaforo(mtxPIDS);
		//sleep(1);
		if(x==1)
			SDL_Delay(1000);
	}
	//kill(monitor,SIGTERM);
	pthread_exit((void *)0); //finaliza el thread
}

void *monitorearEvento(void *) //funcion a utilizar en thread que monitorea archivos que se 
 {        //movieron al directorio enviado por parametro
 int length, i=0;
 char buffer[EVENT_BUF_LEN];
 

 //Crea instancia Inotify
 fd=inotify_init();
 if(fd<0)
 {
  printf("\nError al crear instancia Inotify. Fin del programa\n"); 
  kill(monitor, SIGTERM);
 }
 //se indica el directorio a monitorear
 wd=inotify_add_watch(fd,"./.PID/",IN_CREATE | IN_DELETE | IN_DELETE_SELF);
 if(wd<0)
  {
   printf("\nError en inotify_add_watch. Fin del programa\n");
   kill(monitor, SIGTERM);
  }

 //lee un evento del directorio. Este read es bloqueante hasta que ocurra un evento. Espera pasiva
 while((length=read(fd,buffer,EVENT_BUF_LEN))>0)
  { 
	while(i<length)
	{
	  struct inotify_event *event=(struct inotify_event *)&buffer[i];
	  if(event->len)
	  {
		if(event->mask & IN_ISDIR)
  //en caso de que por error se haya colocado otro nuevo directorio, informa y lo ignora
		printf("\nDirectorio de nombre %s fue movido al directorio monitoreado. Se ignora para su proceso. Solamente procesaremos archivos.\n", event->name);
		else
		{ //en caos de que sea un archivo, lo acola con su path completo
		  leerArchivosPID();
		}
	  }
		i+=EVENT_SIZE + event->len;
	}
	i=0;
   }

 }

 ///////////////////////////////////MAIN//////////////////////////////////////////////

/**************************************************************************************************/

int main() //main. Programa.
{
  	mtxPIDS=obtenerMutex("/mtxPIDS");
	struct sigaction act; //estructura sigaction. Utilizada por el proc. monitor
	memset(&act,0,sizeof(struct sigaction)); //inicializacion para
	//memset(proceso,0,sizeof(t_proceso)*50);
  	if((monitor=fork())==0) //creacion del proceso monitor
  	{
		monitor=getpid();
		int valor_p=0;
	 	act.sa_sigaction=handler; //acciones correspondientes al manejo de la 
		sigfillset(&act.sa_mask); //estructura sigaction. 
		act.sa_flags=SA_SIGINFO; //indicacion de uso de la funcion handler
				 //SIGINT y SIGTERM. Demas senales
   		sigaction(SIGTERM,&act,NULL); //se trabajan con su respuesta por defecto.
   		sigaction(SIGINT,&act,NULL);
   		//sigaction(SIGUSR1,&act,NULL);
   		signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
   		signal(SIGUSR2,SIG_IGN);
	   	sleep(1); //suspension de proceso(para matar a su padre y asi quedar 
		//printf("\n\nIniciando proceso Monitor. PID=%d,PPID=%d\n",getpid(),getppid());
				//como demonio.	
	   	//GUARDO MI IP
		int flag=0;
		FILE * fp = popen("ifconfig", "r");
		if (fp) 
		{
			char *p=NULL, *e; size_t n;
			while ((getline(&p, &n, fp) > 0) && p && flag==0) 
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
							//printf("%s\n", p);
							strcpy(ipmon,p);
							//printf("IP DEL MONITOR: %s_\n", ipmon);
							flag=1;
						}
					}
				}
			}
		}
		pclose(fp);
		leerArchivosPID();
		valor_p = pthread_create(&hilo[0],NULL,monitorearProcesos,NULL);
	   	if(valor_p)
	   	{
		  	printf("Error en la creacion de hilos(threads) de monitorearProcesos. Error devuelto: %d. Fin del programa.  server Partida PID=%d\n",valor_p, getpid());
		 	raise(SIGTERM);  
	   	}  
	   // 	valor_p = pthread_create(&hilo[1],NULL,monitorearEvento,NULL);
	   // 	if(valor_p)
	   // 	{
		  // 	printf("Error en la creacion de hilos(threads) de monitorearEvento. Error devuelto: %d. Fin del programa.  server Partida PID=%d\n",valor_p, getpid());
		 	// raise(SIGTERM);  
	   // 	} 
	   	valor_p = pthread_create(&hilo[1],NULL,actualizarArchivosPID,NULL);
	   	if(valor_p)
	   	{
		  	printf("Error en la creacion de thread de lectura de archivos. Error devuelto: %d.",valor_p);
		 	raise(SIGTERM);  
	   	}
	   	//sleep(20);
	   	//system("./verifica_shm2.sh");
	   	pthread_join(hilo[0], NULL); //aguarda la finalizacion dle thread
	   	//pthread_cancel(hilo[1]);
	   	printf("\nMONITOR: Fin del programa 2\n");
		   //raise(SIGTERM);
		// while(1) //bucle infinito del proceso monitor. Aguarda por una senal enviada por el usuario
		// {			//todo el trabajo lo realizan los threads
		//   	pause();       
		// }
		exit(0); //finaliza
  	}
  	else
  	{
   		if(monitor==-1)
   		{
	 		printf("Error al crear proceso Monitor. Fin\n");
			 exit(0); 
   		}
  	}
 	exit(0); //muerte del proceso padre principal. Fuerza al hijo a quedar como monitor
}
