//INCLUDES 
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <SDL/SDL.h>
	#include <string.h>
	#include <time.h>
	#include <SDL/SDL_ttf.h>
	#include <signal.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <sys/msg.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <string.h>
	#include <sys/msg.h>
	#include <pthread.h>
	#include <stdio.h>
	#include <semaphore.h>
	#include <X11/Xlib.h>
	#include <fcntl.h> 
	#include "./socketsLib/socketsLib.h"
	#include "./semaforosLib/semaforos.h"
	#include "csprite.h"
	#include "keys.h"
	#include "ctimer.h"

//MACROS

	//REPETICION DEL TECLADO
		#define INCIO_REPETICION 50		//SDL_DEFAULT_REPEAT_DELAY
		#define INTERVALO_REPETICION 50	//SDL_DEFAULT_REPEAT_INTERVAL

	//CONFIGURACION DE LA PANTALLA
		#define WIDTH 828	//Ancho de pantalla | Default: 828
		#define HEIGHT 700	//Alto de patalla   | Default: 700
		#define BPP 32		//Profundidad de color
		#define FPS 28

	//COMUNICACIÓN DE EVENTOS
		#define SIN_CAMBIOS 0
		#define GOLPEADO_POR_LLAMA 1
		#define GOLPEADO_POR_BARRIL 2
		#define RESCATO_PRINCESA 3
		#define TIENE_MARTILLO 4
		#define GANO_LA_PARTIDA 5
		#define PERDIO_LA_PARTIDA 6

	//INFORMACIÓN DE LOS SPRITES
		#define CANT_ESCALERAS 12
		#define CANT_ESCALERAS_ROTAS 6
		#define DISTANCIA_ENTRE_PISOS 75 //Default: 75
		#define ALTO_VIGA 27 	//Altura de una viga
		#define ANCHO_VIGA 46 	//Largo de una viga
		#define NO_ESCALERA -1 	//Si mario no está subiendo o bajando una escalera
		#define MAX_BARRILES 50 //La cantidad máxima de barriles en el juego
		#define MAX_VIGAS 200
		#define MAX_LLAMAS 5

	//VELOCIDAD DE MOVIMIENTO
		#define VELOCIDAD 7			//Velocidad al caminar def:5
		#define VELOCIDAD_BARRIL 3 	//Velocidad de los barriles def:3
		#define VELOCIDAD_CAIDA_BARRIL 3 //Velocidad de caída de los barriles def:3
		#define VELOCIDAD_SALTO 3 	//Velocidad al saltar def:3
		#define ALTURA_SALTO 47	//Altura de salto def:50
		#define VELOCIDAD_HORIZONTAL_SALTO 2 //Velocidad horizontal al saltar def:2

	//ANIMACIONES
		#define QUIETO_DER 0	//Mirando hacia la derecha
		#define MIN_DER 1 		//Caminando hacia la derecha
		#define MAX_DER 3
		#define QUIETO_IZQ 4 	//Mirando hacia la izquierda
		#define MIN_IZQ 5 		//Caminando hacia la izquierda
		#define MAX_IZQ 7 
		#define MIN_SALTO_DER 8	//Saltando
		#define MAX_SALTO_DER 8	//En el aire
		#define MIN_SALTO_IZQ 9 //Saltando
		#define MAX_SALTO_IZQ 9 //En el aire
		#define MIN_ESCALERA 10
		#define MAX_ESCALERA 11
		#define MIN_MUERTO	 12
		#define MAX_MUERTO   18

	//ESTADOS DEL JUGADOR
		#define MIRANDO_DER 0		//Mirando hacia la derecha
		#define MIRANDO_IZQ 1		//Mirando hacia la izquierda
		#define DERECHA 2			//Caminando hacia la derecha
		#define IZQUIERDA 3 		//Caminando hacia la izquierda
		#define EN_ESCALERA 4		//Subiendo o bajando una escalera
		#define SALTO_MIRANDO_DER 5	//Saltando hacia la derecha
		#define SALTO_MIRANDO_IZQ 6 //Saltando hacia la izquierda
		#define MUERTO 7			//Mario fue golpeado

	//ESTADOS DE LAS LLAMAS
		#define SUBIENDO_ESCALERA 1
		#define BAJANDO_ESCALERA 2

	//TIMERS
		#define FRECUENCIA_DE_CONTROL_DE_RELOJES 460 //Milisegundos (Precisión de los relojes)
		#define DURACION_MARTILLO 10
		#define TIEMPO_MUERTO 3
		#define DELAY_REINICIO 3

//ESTRUCTURAS

	typedef struct
	{
		int nroJugador;
		int x;
		int y;
		int estado;				//Ver constantes definidas
		int activo;
		int frameActual;		//Frame actual del sprite
		int frameInicial;
		int frameMax;
		int saltando;			//Indica si el jugador está saltando
		int incicioSalto;		//Indica la altura actual (y) al momento de iniciar el salto
		int alturaSaltada;		
		int avanceHorizontal;	//Avance horizontal durante el salto
		int enEscalera;			//Bloquea los movimientos hacia los lados mientras se está subiendo o bajando una escalera
		int maxEscalera;
		int minEscalera;
		int cayendo;			//Indica si mario está en el aire y debe caer
		int cantidadDeRescates;	
		char nombre[50];
		int martillo;
		int evento;
		CTimer tiempoMuerto;
		CTimer relojMartillo;
		//CONTROLES
		int arriba, abajo, izquierda, derecha,  saltar,  salir;
	}t_jugador;

	typedef struct
	{
		int x;
		int y;
		int activo;
		int frameActual;
		int frameInicial;
		int frameMax;
		int sentido;
		int cayendo;
		int enEscalera;
		int estado;
		int minEscalera;
		int nroBarril;
		int recorrido[CANT_ESCALERAS_ROTAS];	//Un espacio para cada escalera rota
	}t_barril;

	typedef struct
	{
		int x;
		int y;
		int frameActual;
		int frameInicial;
		int frameMax;
		int estado;
		int tirandoBarril;
	}t_mono;

	typedef struct
	{
		int x;
		int y;
		int frameActual;
		int frameInicial;
		int frameMax;
		int sentido;
		int cambiarAnimacion;
	}t_princesa;

	typedef struct 
	{
		int x;
		int y;
	}t_viga;

	typedef struct 
	{
		int x;
		int y;
		int frameActual;
		int frameInicial;
		int frameMax;
		int cambiarAnimacion;
	}t_barrilFuego;

	typedef struct 
	{
		int x;
		int y;
		int activo;
		int sentido;
		int enEscalera;
		int estado;
		int minEscalera;
		int maxEscalera;
		int frameActual;
		int frameInicial;
		int frameMax;
		int cambiarAnimacion;
		int nroLlama;
		int recorrido[5];
		int posActual;
	}t_llama;

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
		int estado; 	//0: sin cambios | 1: barril | 2:llama | 3: rescate | 4: martillo
		int barril;
		int recorrido[6];
		int llama;
		int frameActual;
		int sentidoLlama;
	}t_movimientos;

	typedef struct
 	{
      long tipo;
      pid_t pid;
      char nombre[20];
 	}t_proceso;

//PROTOTIPOS DE FUNCIONES

	//Mensajes en pantalla
		void cargarTexto(const char *, int , const char * , SDL_Color , int , int );
		void cargarImagen(const char * , int, int);
		void cargarMensajeDeTriunfo(const t_jugador*);
		void cargarRescateEnPantalla();
		void captarTexto();
		void cargarInicio();
		TTF_Font* cargarFuente(const char*, int);
		void escribirMensaje(const char*, TTF_Font*, SDL_Color, int, int);
		void escribirNombreEnPantalla(const t_jugador*);
		void copiarVector(int*,int*,int);

	//Sistema de juego
		int cargarConfiguracion(const char*,char*, int*, t_jugador*);	//Devuelve el ip en formato cadena, el puerto, y los controles del jugador
		void cargarSprites();				//Se deben cargar todas las imágenes para cada sprite
		void inicializarJuego(int);				//Carga la posición inicial de cada elemento del juego | (0):inicializa la cant de rescates | (!0) conserva la cant de rescates
		void dibujarEscena();					//Dibuja la escena actual
		void manejarEvento(SDL_Event);			//Maneja los eventos que ocurren en el juego
		void finalizarJuego();					//Elimina los sprites
		void verificarColisiones();				//Sistema de colisiones
		void eliminarPID();

	//Vigas
		void dibujarVigas();
		void iniciarVigas();

	//Monos
		void crearMonos();
		void actualizarMonos();

	//Jugadores
		void reiniciarJugador(t_jugador*);
		void caer(t_jugador*);
		void corregirAlturaIzq(t_jugador*);
		void corregirAlturaDer(t_jugador*);
		void moverDerecha(t_jugador*);
		void moverIzquierda(t_jugador*);
		void saltar(t_jugador*);
		void subirEscalera(t_jugador*);
		void bajarEscalera(t_jugador*);
		int verificarEscaleras(t_jugador*);				//Verifica si el jugador se encuentra frente a una escalera
		void actualizarAnimaciones(t_jugador*);			//Actualiza las animaciones del jugador de acuerdo a su estado
		void desactivarMartillo(t_jugador*);
		void activarMartillo(t_jugador*);

	//Barriles
		int crearBarril(int*);
		void dibujarBarriles();
		void moverBarriles();
		void corregirAlturaIzqBarril(t_barril*);
		void corregirAlturaDerBarril(t_barril*);
		void bajarEscaleraBarril(t_barril*);
		int verificarEscalerasBarril(t_barril*);

	//Llamas
		int crearLlama();
		void dibujarLlamas();
		void moverLlamas();
		void corregirAlturaDerLlama(t_llama*);
		void corregirAlturaIzqLlama(t_llama*);
		void usarEscaleraRotaLlama(t_llama*);
		int verificarEscalerasRotasLlama(t_llama*);
		void usarEscaleraLlama(t_llama*);
		int verificarEscalerasLlama(t_llama*);

	//Princesa

		void moverPrincesa();

	//THREADS
		void *conectar_Y_ObtenerDatos(void *param);
		void *verificaMonitor(void *param);
		void *controlarRelojes(void *param);
		void *manejarEventos(void *param);
		void esperarNuevaPartida();

	//HANDLER DE SENALES

		void handler(int iNumSen,siginfo_t *info,void *ni);

//VARIABLES GLOBALES

	//Comunicación
		t_identificacion id_jugador;
		int NRO_JUGADOR;
		int RESCATES=0;
		t_movimientos movimientos, movimientos2;
		char ip[20];
		int puerto;
		FILE *fpid=NULL;
		sem_t *mtxmonitor=NULL;
		int NRO_LLAMA=0;
		int CREAR_LLAMA=0;
		int recorridoBarril[5];
		int recorridoBarril2[5];
		sem_t* mtxPIDS=NULL;
		char DIR_PID[50],DIR_AUX_PID[50], nom_arch[20], comando[50];
		
	//Fuentes
		TTF_Font *fuenteJugador;
		TTF_Font *fuenteTriunfo;

	//Colores
		SDL_Color colorRojo = {255, 0, 0}; //COLOR ROJO
		SDL_Color colorBlanco= {255, 255 ,255}; //COLOR BLANCO
		SDL_Color colorVerde = {0,255,0}; 	//COLOR VERDE
		SDL_Color colorAzul = {0,0,255}; 	//COLOR AZUL

	//Variables simil macros
		int ORIGEN_BARRIL=0;	//Determina qué mono arrojará el barril
		int DIFICULTAD=1;	//0: Fácil | 1: Normal | 2: Difícil
		int VELOCIDAD_HORIZONTAL_LLAMA = DIFICULTAD+1;	//Velocidad de las llamas
		int VELOCIDAD_ESCALERA_LLAMA = DIFICULTAD+1;	//Velocidad de las llamas en escaleras
		int finalizar=0;
		int cantVigas=0;
		int servfd=0;
		int valor_p=0;
		int REINICIANDO=0;

	//Threads
		pthread_t hilo[5];
		pthread_mutex_t movimientos_mtx=PTHREAD_MUTEX_INITIALIZER;

	//Frames

		CFrame Frame;	//Se usa para cargar todas las imágenes

	//Sprites
		CSprite fondo;
		CSprite fondoSuperior;
		CSprite rescateIcono;
		CSprite marioSprite1(19);
		CSprite marioSprite2(19);
		CSprite vigaSprite;
		CSprite barrilSprite(8);
		CSprite monoSpriteIzq(10), monoSpriteDer(10);
		CSprite monoIzqNada(30), monoDerNada(30);
		CSprite princesaSprite(8);
		CSprite barrilFuegoSprite(4);
		CSprite llamaSprite(4);
		CSprite rescateSprite;
		CSprite pilaBarriles1, pilaBarriles2;
		//Vectores
		CSprite* escaleraVec = new CSprite[CANT_ESCALERAS];
		CSprite* escaleraRotaVec = new CSprite[CANT_ESCALERAS_ROTAS];
		CSprite* barrilesVec = new CSprite[MAX_BARRILES];
		CSprite* llamasVec = new CSprite[MAX_LLAMAS];

	//Timers(relojes/temporizadores)
		CTimer tiempoReinicio;

	//Structs
		t_barril barriles[MAX_BARRILES];
		t_mono monoIzq, monoDer;	//Monos
		t_llama llamas[MAX_LLAMAS];
		t_jugador jugador1, jugador2; //Jugadores
		t_jugador jugador[2];
		t_princesa princesa; //Pauline
		t_barrilFuego barrilFuego;
		t_viga vigasVec[MAX_VIGAS];

	//Pantalla

		SDL_Surface *pantalla;

	//Posicion de las escaleras

		//Escaleras normales
		int pos[CANT_ESCALERAS][2]=	//[x][y]
						{//EL ORDEN DE LAS ESCALERAS ES DE IZQUIERDA A DERECHA 
						{103,HEIGHT-DISTANCIA_ENTRE_PISOS-2*ALTO_VIGA+1},            //1 NIVEL, 1RA ESCALERA.
						{WIDTH-127,HEIGHT-DISTANCIA_ENTRE_PISOS-2*ALTO_VIGA+1},		 //1 NIVEL, 2DA ESCALERA.
						{195,HEIGHT-ALTO_VIGA*6.2-DISTANCIA_ENTRE_PISOS-2},			 //2 NIVEL, 1 ESCALERA 
						{332,HEIGHT-ALTO_VIGA*6.2-DISTANCIA_ENTRE_PISOS+7},          //2 NIVEL, 2 ESCALERA 
						{470,HEIGHT-ALTO_VIGA*6.2-DISTANCIA_ENTRE_PISOS+7},		     //2 NIVEL, 3 ESCALERA
						{608,HEIGHT-ALTO_VIGA*6.2-DISTANCIA_ENTRE_PISOS-2},			 //2 NIVEL, 4 ESCALERA
						{103,HEIGHT-ALTO_VIGA*10-DISTANCIA_ENTRE_PISOS+7},		     //3 NIVEL, 1 ESCALERA
						{WIDTH-127,HEIGHT-ALTO_VIGA*10-DISTANCIA_ENTRE_PISOS+7},	 //3 NIVEL, 2 ESCALERA
						{239,HEIGHT-ALTO_VIGA*14-DISTANCIA_ENTRE_PISOS-2},			 //4 NIVEL, 1 ESCALERA
						{WIDTH-264,HEIGHT-ALTO_VIGA*14-DISTANCIA_ENTRE_PISOS-2}, 	 //4 NIVEL, 2 ESCALERA
						{285,HEIGHT-ALTO_VIGA*17.3-DISTANCIA_ENTRE_PISOS-3},		 //5 NIVEL, 1 ESCALERA
						{WIDTH-310,HEIGHT-ALTO_VIGA*17.3-DISTANCIA_ENTRE_PISOS-3} 	 //5 NIVEL, 2 ESCALERA
						};

		//Escaleras rotas
		int posRotas[CANT_ESCALERAS_ROTAS][2]=	//[x][y]
						{//EL ORDEN DE LAS ESCALERAS ES DE IZQUIERDA A DERECHA 
						{5*ANCHO_VIGA+10,HEIGHT-DISTANCIA_ENTRE_PISOS-2*ALTO_VIGA-8},            //1 NIVEL, 1RA ESCALERA.
						{WIDTH-6*ANCHO_VIGA+10,HEIGHT-DISTANCIA_ENTRE_PISOS-2*ALTO_VIGA-8},		 //1 NIVEL, 2DA ESCALERA.
						{4*ANCHO_VIGA+11,HEIGHT-ALTO_VIGA*10-DISTANCIA_ENTRE_PISOS+1},		     //3 NIVEL, 1 ESCALERA
						{WIDTH-5*ANCHO_VIGA+10,HEIGHT-ALTO_VIGA*10-DISTANCIA_ENTRE_PISOS+1},	 //3 NIVEL, 2 ESCALERA
						{3*ANCHO_VIGA+10,HEIGHT-ALTO_VIGA*14-DISTANCIA_ENTRE_PISOS-8},			 //4 NIVEL, 1 ESCALERA
						{WIDTH-4*ANCHO_VIGA+10,HEIGHT-ALTO_VIGA*14-DISTANCIA_ENTRE_PISOS-8} 	 //4 NIVEL, 2 ESCALERA
						};

/**********************************************************************************************************************/
/***********************************************PROGRAMA PRINCIPAL*****************************************************/
/**********************************************************************************************************************/
	
	int main ()
	{
		XInitThreads();//
		mtxPIDS=obtenerMutex("/mtxPIDS");
		
		int tiempoFrame;
		SDL_Event evento;
		
		//CONFIG
		struct sigaction act;
  		memset(&act, 0, sizeof(struct sigaction));

		act.sa_sigaction=handler; //acciones correspondientes al manejo de la 
    			 				//estructura sigaction. 
    	act.sa_flags=SA_SIGINFO; //indicacion de uso de la funcion handler
   				 		//SIGINT y SIGTERM. Demas senales
   		sigaction(SIGTERM,&act,NULL); //se trabajan con su respuesta por defecto.
   		sigaction(SIGINT,&act,NULL);
   		sigaction(SIGPIPE,&act,NULL);
   		//sigaction(SIGUSR1,&act,NULL);
   		signal(SIGCHLD,SIG_IGN); //se ignora la senal SIGCHLD
   		signal(SIGUSR2,SIG_IGN);

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
        fprintf(fpid,"%s\n", "cliente");
        pclose(fp);
   		fclose(fpid);

   		sprintf(comando,"chmod a+r %s",DIR_PID);
   		system(comando);
   		//THREAD QUE RELANZA EL MONITOR SI NO SE ENCUENTRA ACTIVO
    	valor_p = pthread_create(&hilo[2],NULL,verificaMonitor,NULL); //CREA THREAD VERIFICACION
   		if(valor_p)
   		{
      		printf("Error en la creacion de hilos(threads) de verificar Monitor. Error devuelto: %d. Fin del programa\n",valor_p);
     		raise(SIGTERM);  
   		}

		//DIFICULTAD
		srand(time(NULL));
		//
		//atexit(finalizarJuego);
		//
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

		pantalla=SDL_SetVideoMode(WIDTH, HEIGHT, BPP, SDL_HWSURFACE|SDL_ANYFORMAT|SDL_DOUBLEBUF);
		if(pantalla==NULL)
		{
		    printf("No se ha podido establecer el modo de vídeo: %s\n", SDL_GetError());
		    exit(1);
		}
		if(cargarConfiguracion("config.cfg",ip,&puerto,&jugador[0])<0)
		{
			printf("Error al cargar la configuración del usuario\n");
			exit(1);
		}
		if(cargarConfiguracion("config.cfg",ip,&puerto,&jugador[1])<0)
		{
			printf("Error al cargar la configuración del usuario\n");
			exit(1);
		}
		SDL_WM_SetCaption("Ya Fuiste - Donkey Kong",NULL);

		//INFORMACIÓN DE CONFIGURACIÓN
		printf("PID: %d\n",getpid());
		printf("Conexión:\n\tIP: %s\tPUERTO: %d\n",ip,puerto);
		printf("Controles:\n\t%s\t%s\t%s\t%s\t%s\t%s\n",SDL_GetKeyName((SDLKey)jugador[0].arriba), SDL_GetKeyName((SDLKey)jugador[0].abajo),SDL_GetKeyName((SDLKey)jugador[0].izquierda),SDL_GetKeyName((SDLKey)jugador[0].derecha),SDL_GetKeyName((SDLKey)jugador[0].saltar),SDL_GetKeyName((SDLKey)jugador[0].salir));

		cargarInicio(); //SE INGRESA EL NOMBRE DEL JUGADOR

		//SE CONECTA AL SERVIDOR DE TORNEO RECIBE LA INFORMACIÓN PARA INICIAR EL JUEGO
		valor_p = pthread_create(&hilo[0],NULL,conectar_Y_ObtenerDatos,NULL);
  		if(valor_p)
   		{
     		printf("Error en la creacion de hilos(threads) de conexion y obtencion de datos. Error devuelto: %d. Fin del programa\n",valor_p);
   		    raise(SIGTERM);  
 		}
		pthread_join(hilo[0],NULL);
	
		cargarSprites();
		inicializarJuego(0);
		SDL_EnableKeyRepeat(INCIO_REPETICION,INTERVALO_REPETICION);
		dibujarEscena();
		int error;

		//SE INICIA EL THREAD DE CONTROL DE RELOJES
		valor_p = pthread_create(&hilo[1],NULL,controlarRelojes,NULL);
  		if(valor_p)
   		{
     		printf("Error en la creacion del hilo(thread) de control de relojes (timers). Error devuelto: %d. Fin del programa\n",valor_p);
   		    raise(SIGTERM);  
 		}

 		//SE INICIA EL THREAD DE MANEJO DE EVENTOS (TECLADO)
		// valor_p = pthread_create(&hilo[2],NULL,manejarEventos,NULL);
		// if(valor_p)
		// {
		// 	printf("Error en la creacion del hilo(thread) de manejo de eventos. Error devuelto: %d. Fin del programa\n",valor_p);
		// 	aise(SIGTERM);  
		// }

		//Bucle principal del juego

 		CREAR_LLAMA=0;

 		//RELLENO LA ESTRUCTURA DE COMUNICACIÓN
				//pthread_mutex_lock(&movimientos_mtx);
				movimientos.x=jugador[NRO_JUGADOR].x;
				movimientos.y=jugador[NRO_JUGADOR].y;
				movimientos.numero=NRO_JUGADOR;
				movimientos.frameActual=jugador[NRO_JUGADOR].frameActual;
				movimientos.llama=CREAR_LLAMA;
				movimientos.estado=jugador[NRO_JUGADOR].evento;
				movimientos.barril=0;
		//ENVÍO MI INFORMACIÓN AL SERVIDOR
				if(enviarDatos(&servfd,&movimientos,sizeof(t_movimientos))==-1)
	         		printf("\nError al enviar paquete de datos a Servidor Partida.\n");

		while(finalizar==0)
		{
			tiempoFrame = SDL_GetTicks();	//Tiempo de inicio del frame
			//SI GANAMOS, ESPERAMOS UNA NUEVA PPARTIDA
			if(jugador[NRO_JUGADOR].evento==GANO_LA_PARTIDA && finalizar==0)
			{
				cargarRescateEnPantalla();
				printf("Esperando nuevo oponente\n");
				esperarNuevaPartida();
				finalizar=0; error=0;
			}
			else
			{
				error=0;
				//bzero(&movimientos,sizeof(t_movimientos));
				if(SDL_PollEvent(&evento))		//Verificamos si ocurre un evento en el juego
				{
					manejarEvento(evento);
				}
	         	//RECIBO LA INFORMACIÓN DEL OTRO JUGADOR
	         	bzero(&movimientos,sizeof(t_movimientos));
				if(recibirDatos(&servfd,&movimientos,sizeof(t_movimientos))==-1)
				{
				 	printf("\nError al recibir paquete de datos del Servidor de Partida.\n");
				 	error=1;
				}
				if(error==0)
				{
					if(movimientos.numero==0 && movimientos.x==0 && movimientos.y==0 && movimientos.estado!=GANO_LA_PARTIDA)
					{
						printf("%s abandonó la partida\n",NRO_JUGADOR==0?jugador[1].nombre:jugador[0].nombre);
						jugador[NRO_JUGADOR].cantidadDeRescates=RESCATES;
						jugador[NRO_JUGADOR].evento=GANO_LA_PARTIDA;
						cargarRescateEnPantalla();
					}
					if(movimientos.estado==PERDIO_LA_PARTIDA)
					{
						if(NRO_JUGADOR==0)
						{
							printf("PERDÍ!!!!\n");
							jugador[1].cantidadDeRescates=RESCATES;
							jugador[1].evento=GANO_LA_PARTIDA;
							cargarRescateEnPantalla();
							sleep(2);
							raise(SIGTERM);
						}
						else
							{
								printf("PERDÍ!!!!\n");
								jugador[0].cantidadDeRescates=RESCATES;
								jugador[0].evento=GANO_LA_PARTIDA;
								cargarRescateEnPantalla();
								sleep(2);
								raise(SIGTERM);
							}
					}
					else
						if(movimientos.estado==GANO_LA_PARTIDA)
						{
							jugador[NRO_JUGADOR].cantidadDeRescates=RESCATES;
							jugador[NRO_JUGADOR].evento=GANO_LA_PARTIDA;
							cargarRescateEnPantalla();
							printf("Esperando nuevo oponente\n");
							esperarNuevaPartida();
							printf("NO DEBERIA HABER LLEGADO ACA\n");
							finalizar=0; error=0;
						}
					jugador[movimientos.numero].x=movimientos.x;
					jugador[movimientos.numero].y=movimientos.y;
					jugador[movimientos.numero].frameActual=movimientos.frameActual;

					//Lanzamiento de barriles
					if(movimientos.barril==1)
					{
						if(ORIGEN_BARRIL%2==0)
						{
							if(monoDer.tirandoBarril==0)
							{
								copiarVector(recorridoBarril,movimientos.recorrido,CANT_ESCALERAS_ROTAS);
								monoDer.tirandoBarril=1;
								monoDer.frameInicial=0;
								monoDer.frameActual=0;
								monoDer.estado=0;
								monoDer.frameMax=9;
							}
						}
						else
						{
							if(monoIzq.tirandoBarril==0)
							{
								copiarVector(recorridoBarril2,movimientos.recorrido,CANT_ESCALERAS_ROTAS);
								monoIzq.tirandoBarril=1;
								monoIzq.frameInicial=0;
								monoIzq.frameActual=0;
								monoIzq.estado=0;
								monoIzq.frameMax=9;
							}
						}
					}
					//Creación de llamas
					if(movimientos.llama==2)
					{
						crearLlama();
					}
				}
				//pthread_mutex_unlock(&movimientos_mtx);
				CREAR_LLAMA=0;
				//ACCIONES PERIÓDICAS
				if(jugador[NRO_JUGADOR].evento!=GANO_LA_PARTIDA && jugador[NRO_JUGADOR].evento!=PERDIO_LA_PARTIDA)
				{
					saltar(&jugador[NRO_JUGADOR]);
					jugador[NRO_JUGADOR].evento=0;
					dibujarEscena();
					//Movimientos automáticos
					moverBarriles();
					moverLlamas();
					moverPrincesa();
					//Verificación de colisiones
					verificarColisiones();
				}	
				//RELLENO LA ESTRUCTURA DE COMUNICACIÓN
					//pthread_mutex_lock(&movimientos_mtx);
					movimientos.x=jugador[NRO_JUGADOR].x;
					movimientos.y=jugador[NRO_JUGADOR].y;
					movimientos.numero=NRO_JUGADOR;
					movimientos.frameActual=jugador[NRO_JUGADOR].frameActual;
					movimientos.llama=CREAR_LLAMA;
					movimientos.estado=jugador[NRO_JUGADOR].evento;
					movimientos.barril=0;
					// if(movimientos.estado!=0)
					// 	printf("Envío: %d\n",movimientos.estado);
					//ENVÍO MI INFORMACIÓN AL SERVIDOR
					if(enviarDatos(&servfd,&movimientos,sizeof(t_movimientos))==-1)
		         		printf("\nError al enviar paquete de datos a Servidor Partida.\n");	
			}
			tiempoFrame = SDL_GetTicks()-tiempoFrame;	//Tiempo de finalización del frame
			if(tiempoFrame < 1000/FPS)
	    		SDL_Delay(Uint32(1000/FPS-tiempoFrame));
		}
		finalizarJuego();
		return 0;
	}

/**********************************************************************************************************************/
/***********************************************SISTEMA DE JUEGO*******************************************************/
/**********************************************************************************************************************/

	int cargarConfiguracion(const char*path ,char* ip, int* puerto, t_jugador*jugador)
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
		//IP
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux++;
		sscanf(aux,"%s",ip);
		//PUERTO
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux++;
		sscanf(aux,"%d",puerto);
		//SUBIR
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->arriba=getSDLkey(valor);
		if(jugador->arriba<0)
			return -1;
		//BAJAR
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->abajo=getSDLkey(valor);
		if(jugador->abajo<0)
			return -1;
		//IZQUIERDA
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->izquierda=getSDLkey(valor);
		if(jugador->izquierda<0)
			return -1;
		//DERECHA
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->derecha=getSDLkey(valor);
		if(jugador->derecha<0)
			return -1;
		//SALTAR
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->saltar=getSDLkey(valor);
		if(jugador->saltar<0)
			return -1;
		//SALIR
		fgets(buffer, sizeof(buffer),pf);
		aux=strrchr(buffer,'\n');
	    if (aux)
	        *aux='\0';
		aux=strrchr(buffer,':');
		aux+=2;
		sscanf(aux,"%s",valor);
		jugador->salir=getSDLkey(valor);
		if(jugador->salir<0)
			return -1;
		fclose(pf);
		return 0;
	}

	/**********************************************************************************************************************/

	void inicializarJuego(int cod)
	{
		jugador[0].nroJugador=0;
		jugador[0].x=10;
		jugador[0].y=HEIGHT-marioSprite1.geth()-ALTO_VIGA*2+6;
		jugador[0].estado=MIRANDO_DER;
		jugador[0].saltando=0;
		jugador[0].incicioSalto=0;
		jugador[0].alturaSaltada=0;
		jugador[0].avanceHorizontal=0;
		jugador[0].enEscalera=-1;
		jugador[0].cayendo=0;
		jugador[0].activo=1;
		jugador[0].martillo=0;
		jugador[0].evento=SIN_CAMBIOS;
		actualizarAnimaciones(&jugador[0]);
		//
		jugador[1].nroJugador=1;
		jugador[1].x=WIDTH - marioSprite2.getw()-10;
		jugador[1].y=HEIGHT-marioSprite2.geth()-ALTO_VIGA*2+6;
		jugador[1].estado=MIRANDO_IZQ;
		jugador[1].saltando=0;
		jugador[1].incicioSalto=0;
		jugador[1].alturaSaltada=0;
		jugador[1].avanceHorizontal=0;
		jugador[1].enEscalera=-1;
		jugador[1].cayendo=0;
		jugador[1].activo=1;
		jugador[1].martillo=0;
		jugador[1].evento=0;
		actualizarAnimaciones(&jugador[1]);
		//
		princesa.x=300;
		princesa.y=HEIGHT-ALTO_VIGA*18.5-DISTANCIA_ENTRE_PISOS;
		princesa.frameInicial=0;
		princesa.frameMax=3;
		princesa.frameActual=0;
		princesa.sentido=1;
		princesa.cambiarAnimacion=0;
		//
		crearMonos();
		//
		barrilFuego.x=WIDTH/2-barrilFuegoSprite.getw()/2;
		barrilFuego.y=HEIGHT - vigaSprite.geth()-barrilFuegoSprite.geth();
		barrilFuegoSprite.setx(barrilFuego.x);
		barrilFuegoSprite.sety(barrilFuego.y);
		barrilFuego.frameInicial=0;
		barrilFuego.frameMax=3;
		barrilFuego.frameActual=0;
		barrilFuego.cambiarAnimacion=0;
		//
		pilaBarriles1.setx(0);
		pilaBarriles1.sety(164);
		pilaBarriles2.setx(WIDTH - pilaBarriles1.getw());
		pilaBarriles2.sety(164);
	 	//
		for(int i=0;i<MAX_VIGAS;i++)
		{
			vigasVec[i].x=WIDTH;
			vigasVec[i].y=HEIGHT;
		}
		for(int i=0;i<MAX_BARRILES;i++)
		{
			barriles[i].activo=0;
			barriles[i].frameActual=0;
			barriles[i].frameInicial=0;
			barriles[i].frameMax=7;
			barriles[i].sentido=1;
			barriles[i].cayendo=0;
			barriles[i].x=WIDTH;
			barriles[i].y=HEIGHT;
			barriles[i].enEscalera=NO_ESCALERA;
			barriles[i].estado=0;
			barriles[i].nroBarril=i;
		}
		for(int i=0;i<MAX_LLAMAS;i++)
		{
			llamas[i].activo=0;
			llamas[i].frameActual=0;
			llamas[i].frameInicial=0;
			llamas[i].frameMax=3;
			llamas[i].sentido=1;
			llamas[i].x=WIDTH;
			llamas[i].y=HEIGHT;
			llamas[i].enEscalera=NO_ESCALERA;
			llamas[i].estado=0;
			llamas[i].nroLlama=i;
			llamas[i].cambiarAnimacion=0;
			llamas[i].posActual=0;
		}
		iniciarVigas();
		if(cod==0)
		{
			//Borramos la franja superior de la pantalla
			//fondoSuperior.dibujar(pantalla);
			//Dibujamos los nombres de los jugadores
			ORIGEN_BARRIL=0;
			NRO_LLAMA=0;
			escribirNombreEnPantalla(&jugador[0]);
			escribirNombreEnPantalla(&jugador[1]);
			jugador[0].cantidadDeRescates=0;
			jugador[1].cantidadDeRescates=0;
		}
	}

	/**********************************************************************************************************************/

	void cargarSprites()
	{
		//Fuentes
		fuenteJugador=cargarFuente("fonts/Jumpman.ttf",20);
		fuenteTriunfo=cargarFuente("fonts/Jumpman.ttf",60);
		//Fondo superior
		Frame.cargar("img/fondo_superior.bmp");
		fondoSuperior.agregarFrame(Frame);
		//Fondo inferior
		Frame.cargar("img/fondo_inferior.bmp");
		fondo.agregarFrame(Frame);
		fondo.sety(75);
		fondo.setx(0);
		//Icono de rescate
		Frame.cargar("img/princesa/princesa_rescate.bmp");
		rescateIcono.agregarFrame(Frame);
		//Fondo rescate
		Frame.cargar("img/marioRescate.bmp");
		rescateSprite.agregarFrame(Frame);
		//Mario 1
		marioSprite1.cargarCarpeta("img/mario/mario_",19);
		//Mario 2
		marioSprite2.cargarCarpeta("img/luigi/mario_",19);
		//Viga
		Frame.cargar("img/objetos/vigaAngosta.bmp");
		vigaSprite.agregarFrame(Frame);
		//Barril
		barrilSprite.cargarCarpeta("img/barril/barril_",8);
		Frame.cargar("img/barril/barril_0.bmp");
		for(int i=0;i<MAX_BARRILES;i++)
		{
			barrilesVec[i].agregarFrame(Frame);
			barrilesVec[i].selframe(0); 
		}
		//Monos
		monoSpriteIzq.cargarCarpeta("img/mono_izq/mono_",10);
		monoSpriteDer.cargarCarpeta("img/mono_der/mono_",10);
		//Monos cuando no tiran barriles
		monoIzqNada.cargarCarpeta("img/mono_activoIzq/mono_",30);
		monoDerNada.cargarCarpeta("img/mono_activoDer/mono_",30);
		//Princesa
		princesaSprite.cargarCarpeta("img/princesa/princesa_",8);
		//Barril de fuego
		barrilFuegoSprite.cargarCarpeta("img/barril_fuego/barril_fuego_",4);
		barrilFuegoSprite.selframe(0);
		//Pila de barriles
		Frame.cargar("img/objetos/pila_de_barriles.bmp");
		pilaBarriles1.agregarFrame(Frame);
		pilaBarriles2.agregarFrame(Frame);
		//Llama de fuego
	 	llamaSprite.cargarCarpeta("img/llama/llama_",4);
	 	Frame.cargar("img/llama/llama_0.bmp");
	 	for(int i=0;i<MAX_LLAMAS;i++)
		{
			llamasVec[i].agregarFrame(Frame);
			llamasVec[i].selframe(0);
		}
		//Escaleras
		Frame.cargar("img/objetos/escalera.bmp");
		for(int i=0;i<2;i++)
		{
			escaleraVec[i].agregarFrame(Frame);
			escaleraVec[i].selframe(0);
		}

		escaleraVec[6].agregarFrame(Frame);
		escaleraVec[6].selframe(0);
		escaleraVec[7].agregarFrame(Frame);
		escaleraVec[7].selframe(0);

		Frame.cargar("img/objetos/escalera1.bmp");
		escaleraVec[2].agregarFrame(Frame);
		escaleraVec[2].selframe(0);
		escaleraVec[5].agregarFrame(Frame);
		escaleraVec[5].selframe(0);

		Frame.cargar("img/objetos/escalera2.bmp");
		escaleraVec[8].agregarFrame(Frame);
		escaleraVec[8].selframe(0);
		escaleraVec[9].agregarFrame(Frame);
		escaleraVec[9].selframe(0);

		Frame.cargar("img/objetos/escalera3.bmp");
		escaleraVec[3].agregarFrame(Frame);
		escaleraVec[3].selframe(0);
		escaleraVec[4].agregarFrame(Frame);
		escaleraVec[4].selframe(0);
		escaleraVec[10].agregarFrame(Frame);
		escaleraVec[10].selframe(0);
		escaleraVec[11].agregarFrame(Frame);
		escaleraVec[11].selframe(0);

		for(int i=0;i<CANT_ESCALERAS;i++)
		{
			escaleraVec[i].setx(pos[i][0]);
			escaleraVec[i].sety(pos[i][1]);
		}

		//Escaleras rotas
		Frame.cargar("img/objetos/escalera_rota_0.bmp");
		escaleraRotaVec[0].agregarFrame(Frame);
		escaleraRotaVec[0].selframe(0);
		escaleraRotaVec[1].agregarFrame(Frame);
		escaleraRotaVec[1].selframe(0);
		Frame.cargar("img/objetos/escalera_rota_1.bmp");
		escaleraRotaVec[2].agregarFrame(Frame);
		escaleraRotaVec[2].selframe(0);
		escaleraRotaVec[3].agregarFrame(Frame);
		escaleraRotaVec[3].selframe(0);
		Frame.cargar("img/objetos/escalera_rota_2.bmp");
		escaleraRotaVec[4].agregarFrame(Frame);
		escaleraRotaVec[4].selframe(0);
		escaleraRotaVec[5].agregarFrame(Frame);
		escaleraRotaVec[5].selframe(0);
		for(int i=0;i<CANT_ESCALERAS_ROTAS;i++)
		{
			escaleraRotaVec[i].setx(posRotas[i][0]);
			escaleraRotaVec[i].sety(posRotas[i][1]);
		}
	}

	/**********************************************************************************************************************/
	void dibujarEscena()
	{
		if(REINICIANDO==1)
			return;
		int i;
		//Borramos la pantalla
		SDL_Rect borrar;
		borrar.x=0;
		borrar.y=75;
		borrar.w=WIDTH;
		borrar.h=HEIGHT-75;
		SDL_FillRect(pantalla, &borrar, SDL_MapRGB(pantalla->format, 0, 0, 0));
		//Dibujamos el fondo
		//fondo.dibujar(pantalla); //CONSUME MUCHO CPU
		//Dibujar vigas
		dibujarVigas();
		//Dibujar escaleras
		for(i=0;i<CANT_ESCALERAS;i++)
			escaleraVec[i].dibujar(pantalla);
		//Dibujar escaleras rotas
		for(i=0;i<CANT_ESCALERAS_ROTAS;i++)
			escaleraRotaVec[i].dibujar(pantalla);
		//Verificar caída
		if(jugador[0].saltando==0)
		 	caer(&jugador[0]);
		//Dibujar jugador 1
		marioSprite1.setx(jugador[0].x);
		marioSprite1.sety(jugador[0].y);
		marioSprite1.selframe(jugador[0].frameActual);	//Seleccionamos el frame a dibujar
		marioSprite1.dibujar(pantalla);	//Dibujamos el frame
		//Dibujar jugador 2
		if(jugador[1].saltando==0)
		  	caer(&jugador[1]);

		marioSprite2.setx(jugador[1].x);
		marioSprite2.sety(jugador[1].y);
		marioSprite2.selframe(jugador[1].frameActual);	//Seleccionamos el frame a dibujar
		marioSprite2.dibujar(pantalla);	//Dibujamos el frame
		//Dibujar princesa
		princesaSprite.setx(princesa.x);
		princesaSprite.sety(princesa.y);
		princesaSprite.selframe(princesa.frameActual);	//Seleccionamos el frame a dibujar
		princesaSprite.dibujar(pantalla);
		//Dibujar Monos
		actualizarMonos();
		//Dibujar barril de fuego
		barrilFuegoSprite.selframe(barrilFuego.frameActual);
		barrilFuegoSprite.dibujar(pantalla);
		if(barrilFuego.cambiarAnimacion==3)
		{
	    	barrilFuego.cambiarAnimacion=0;
			barrilFuego.frameActual++;
			if(barrilFuego.frameActual>barrilFuego.frameMax)
				barrilFuego.frameActual=barrilFuego.frameInicial;
		}
		else
			barrilFuego.cambiarAnimacion++;
		//Dibujar pilas de barriles
		pilaBarriles1.dibujar(pantalla);
		pilaBarriles2.dibujar(pantalla);
		//Dibujar barriles
		dibujarBarriles();
		//Dibujar llamas
		dibujarLlamas();
		//Transferir cambios
		SDL_Flip(pantalla);
	}

	/**********************************************************************************************************************/

	void manejarEvento(SDL_Event evento)
	{
		Uint8 *keys;
		switch(evento.type)
		{
			case SDL_QUIT:
							finalizar=1;
							//finalizarJuego();
							break;
			case SDL_KEYDOWN:
							keys = SDL_GetKeyState(NULL);
							//TECLA PARA SALIR DEL JUEGO
							if(keys[jugador[NRO_JUGADOR].salir])
							{
								finalizar=1;
								//finalizarJuego();
								break;
							}
							//JUGADOR 1
							if(jugador[NRO_JUGADOR].activo==0)
								break;
							if(keys[jugador[NRO_JUGADOR].saltar] && jugador[NRO_JUGADOR].y > 0 && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0)
							{
								if(jugador[NRO_JUGADOR].saltando==0)	
								{
									jugador[NRO_JUGADOR].saltando=1;	//BLOQUEAMOS LOS DEMÁS MOVIMIENTOS
									jugador[NRO_JUGADOR].incicioSalto=jugador[NRO_JUGADOR].y;
									jugador[NRO_JUGADOR].alturaSaltada=0;
									jugador[NRO_JUGADOR].avanceHorizontal=0;	//SALTO FIJO
									if(keys[jugador[NRO_JUGADOR].derecha])
									{
										jugador[NRO_JUGADOR].avanceHorizontal=VELOCIDAD_HORIZONTAL_SALTO;	//SALTO HACIA DERECHA
										jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
									}
									else
										if(keys[jugador[NRO_JUGADOR].izquierda])
										{
											jugador[NRO_JUGADOR].avanceHorizontal=(-VELOCIDAD_HORIZONTAL_SALTO);	//SALTO HACIA IZQUIERDA
											jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
										}
									switch(jugador[NRO_JUGADOR].estado)
									{
										case MIRANDO_DER:
														jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
														break;
										case MIRANDO_IZQ:
														jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
														break;
										case DERECHA:
														jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
														break;
										case IZQUIERDA:
														jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
														break;
									}
								}	
							}
							if(keys[SDLK_r])
							{
								jugador[NRO_JUGADOR].cantidadDeRescates++;	
							}

							if(keys[jugador[NRO_JUGADOR].arriba] && jugador[NRO_JUGADOR].y>0)
							{
								subirEscalera(&jugador[NRO_JUGADOR]);
							}
							if(keys[jugador[NRO_JUGADOR].abajo] && jugador[NRO_JUGADOR].y < HEIGHT - marioSprite1.geth() && jugador[NRO_JUGADOR].cayendo==0) 
							{
								bajarEscalera(&jugador[NRO_JUGADOR]);
							}
							if(keys[jugador[NRO_JUGADOR].izquierda] && jugador[NRO_JUGADOR].x > 0 && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0) 	//MOVERSE A LA IZQUIERDA
							{
								moverIzquierda(&jugador[NRO_JUGADOR]);
							}
							if(keys[jugador[NRO_JUGADOR].derecha] && jugador[NRO_JUGADOR].x < WIDTH - marioSprite1.getw() && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0) 	//MOVERSE A LA DERECHA
							{
								moverDerecha(&jugador[NRO_JUGADOR]);
							}

							//CREAR BARRILES MANUALMENTE
							if(keys[SDLK_b])
							{
								if(crearBarril(id_jugador.recorrerllama1)<0)
									printf("Límite de barriles alcanzado\n");
								else
									printf("Barril creado\n");
							}

							if(keys[SDLK_l])
							{
								if(crearLlama()<0)
									printf("Límite de llamas alcanzado\n");
								else
									printf("Llama creada\n");
							}
							break;

			case SDL_KEYUP:	//En caso de que no se ejecuten acciones, reiniciamos la animación al estado de espera
							if(jugador[NRO_JUGADOR].cayendo==0)
							{
								switch(jugador[NRO_JUGADOR].estado)
								{
									case DERECHA:
													if(jugador[NRO_JUGADOR].avanceHorizontal>0)
													{
														jugador[NRO_JUGADOR].estado=MIRANDO_DER;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
													}
													else
													{
														jugador[NRO_JUGADOR].frameActual=QUIETO_DER;
													}
													break;
									case IZQUIERDA:
													if(jugador[NRO_JUGADOR].avanceHorizontal>0)
													{
														jugador[NRO_JUGADOR].estado=MIRANDO_IZQ;
														actualizarAnimaciones(&jugador[NRO_JUGADOR]);
													}
													else
													{
														jugador[NRO_JUGADOR].frameActual=QUIETO_IZQ;
													}
													break;
									default:
													// jugador[NRO_JUGADOR].estado=MIRANDO_DER;
													// actualizarAnimaciones(&jugador[NRO_JUGADOR]);
													break;
								}
							}
		}
	}

	/**********************************************************************************************************************/

	void verificarColisiones()
	{
		//RESCATE DE LA PRINCESA Y COLISION CON BARRIL DE FUEGO	
		if(jugador[0].activo==1)
		{
			//JUGADOR 1 RESCATA A LA PRINCESA
			if(princesaSprite.colision(marioSprite1))
			{

				REINICIANDO=1;
				jugador[0].activo=0;
				jugador[1].activo=0;
				jugador[0].cantidadDeRescates++;
				jugador[0].evento=RESCATO_PRINCESA;
				printf("%s rescató a la princesa!\n",jugador[0].nombre);
				//printf("Cantidad de rescates: %d\n",jugador[0].cantidadDeRescates);
				printf("Reiniciando juego\n");
				SDL_Rect borrar;
				borrar.x=0;
				borrar.y=0;
				borrar.w=WIDTH;
				borrar.h=HEIGHT;
				SDL_FillRect(pantalla,NULL, SDL_MapRGB(pantalla->format, 0, 0, 0));
				rescateSprite.setx(358);
				rescateSprite.sety(150);
				rescateSprite.dibujar(pantalla);
				char auxCad[200];
				sprintf(auxCad,"%s ha rescatado a la princesa!",jugador[0].nombre);
				escribirMensaje(auxCad,fuenteJugador,colorVerde,250,400);
				SDL_Flip(pantalla);	
				tiempoReinicio.iniciar(DELAY_REINICIO);	
				return;
			}
			//COLISIÓN CON BARRIL DE FUEGO (BLOQUEO DEL PASO)
			if(barrilFuegoSprite.colision(marioSprite1))
			{
				if(jugador[0].estado==DERECHA || jugador[0].estado==SALTO_MIRANDO_DER)
					jugador[0].x-=VELOCIDAD;
				else
					jugador[0].x+=VELOCIDAD;
			}
		}

		if(jugador[1].activo==1)
		{
			//JUGADOR 2 RESCATA A LA PRINCESA
			if(princesaSprite.colision(marioSprite2))
			{
				REINICIANDO=1;
				jugador[0].activo=0;
				jugador[1].activo=0;
				jugador[1].cantidadDeRescates++;
				jugador[1].evento=RESCATO_PRINCESA;
				printf("%s ha rescatado a la princesa!\n",jugador[1].nombre);
				//printf("Cantidad de rescates: %d\n",jugador[1].cantidadDeRescates);
				printf("Reiniciando juego\n");
				SDL_Rect borrar;
				borrar.x=0;
				borrar.y=0;
				borrar.w=WIDTH;
				borrar.h=HEIGHT;
				SDL_FillRect(pantalla,NULL, SDL_MapRGB(pantalla->format, 0, 0, 0));
				rescateSprite.setx(358);
				rescateSprite.sety(150);
				rescateSprite.dibujar(pantalla);
				char auxCad[200];
				sprintf(auxCad,"%s ha rescatado a la princesa!",jugador[1].nombre);
				escribirMensaje(auxCad,fuenteJugador,colorVerde,250,400);	
				SDL_Flip(pantalla);	
				tiempoReinicio.iniciar(DELAY_REINICIO);			
				return;
			}
			//COLISIÓN CON BARRIL DE FUEGO (BLOQUEO DEL PASO)
			if(barrilFuegoSprite.colision(marioSprite2))
			{
				if(jugador[1].estado==DERECHA || jugador[1].estado==SALTO_MIRANDO_DER)
					jugador[1].x-=VELOCIDAD;
				else
					jugador[1].x+=VELOCIDAD;
			}
		}
		
		//COLISIONES QUE INVOLUCRAN BARRILES
		for(int i=0;i<MAX_BARRILES;i++)
		{
			if(barriles[i].activo==1)
			{
				barrilSprite.setx(barriles[i].x);
				barrilSprite.sety(barriles[i].y);
				//Barril choca con barril de fuego
				if(barrilSprite.colision(barrilFuegoSprite))
				{
					barriles[i].activo=0;
					CREAR_LLAMA=1;
				}
				//Barril choca con Jugador 1
				if(jugador[0].activo==1)
				{
					if(barrilSprite.colision(marioSprite1))
					{
						printf("%s fue golpeado por un barril!\n",jugador[0].nombre);	
						printf("Reiniciando jugador 1\n");
						jugador[0].evento=GOLPEADO_POR_BARRIL;
						jugador[0].activo=0;
						jugador[0].estado=MUERTO;
						actualizarAnimaciones(&jugador[0]);
						jugador[0].tiempoMuerto.iniciar(TIEMPO_MUERTO);
						return;
					}
				}
				//Barril choca con Jugador 2
				if(jugador[1].activo==1)
				{
					if(barrilSprite.colision(marioSprite2))
					{
						printf("%s fue golpeado por un barril!\n",jugador[1].nombre);
						printf("Reiniciando jugador 2\n");
						jugador[1].evento=GOLPEADO_POR_BARRIL;
						jugador[1].activo=0;
						jugador[1].estado=MUERTO;
						actualizarAnimaciones(&jugador[1]);
						jugador[1].tiempoMuerto.iniciar(TIEMPO_MUERTO);
						return;
					}
				}
			}
		}

		//COLISIONES CON LLAMAS
		for(int i=0;i<MAX_LLAMAS;i++)
		{
			if(llamas[i].activo==1)
			{
				llamaSprite.setx(llamas[i].x);
				llamaSprite.sety(llamas[i].y);
				//Llama choca con Jugador 1
				if(jugador[0].activo==1)
				{
					if(llamaSprite.colision(marioSprite1))
					{
						printf("%s fue golpeado por una llama!\n",jugador[0].nombre);
						printf("Reiniciando jugador 1\n");
						jugador[0].evento=GOLPEADO_POR_LLAMA;
						jugador[0].activo=0;
						jugador[0].estado=MUERTO;
						actualizarAnimaciones(&jugador[0]);
						jugador[0].tiempoMuerto.iniciar(TIEMPO_MUERTO);
						return;
					}
				}
				//Llama choca con Jugador 2
				if(jugador[1].activo==1)
				{
					if(llamaSprite.colision(marioSprite2))
					{
						printf("%s fue golpeado por una llama!\n",jugador[1].nombre);
						printf("Reiniciando jugador 2\n");
						jugador[1].evento=GOLPEADO_POR_LLAMA;
						jugador[1].activo=0;
						jugador[1].estado=MUERTO;
						actualizarAnimaciones(&jugador[1]);
						jugador[1].tiempoMuerto.iniciar(TIEMPO_MUERTO);
						return;
					}
				}
			}
		}
	}

	/**********************************************************************************************************************/

	void finalizarJuego()
	{
		marioSprite1.finalizar();
		marioSprite2.finalizar();
		vigaSprite.finalizar();
		princesaSprite.finalizar();
		delete[] escaleraVec;
		delete[] escaleraRotaVec;
		delete[] barrilesVec;
		delete[] llamasVec;
		barrilSprite.finalizar();
		monoSpriteIzq.finalizar();
		monoSpriteDer.finalizar();
		monoDerNada.finalizar();
		monoIzqNada.finalizar();
		barrilFuegoSprite.finalizar();
		llamaSprite.finalizar();
		rescateSprite.finalizar();
		fondo.finalizar();
		rescateIcono.finalizar();
		SDL_FreeSurface(pantalla);
		TTF_CloseFont(fuenteJugador);
		TTF_CloseFont(fuenteTriunfo);
		//ELIMINAMOS EL PID DE LA LISTA DE PIDS
		eliminarPID();
		TTF_Quit();
		SDL_Quit();
		printf("FIN DEL PROGRAMA\n");
	}

	/**********************************************************************************************************************/

	void copiarVector(int*vec1, int*vec2, int tam)
	{
		for(int i=0;i<tam;i++)
			vec1[i]=vec2[i];
	}

	/**********************************************************************************************************************/

	void eliminarPID()
	{
		
		pedirSemaforo(mtxPIDS);
        char comando2[50], comando3[50];
        bzero(comando2,sizeof(comando2));
        sprintf(comando2,"./.PID/%s",nom_arch);
        //sprintf(comando3,"mv ./.aux/%s2 ./.PID/%s",nom_arch, nom_arch);
        remove(comando2);
        //system(comando3);
        //system(comando);
        //printf("ELIMINÉ MI PID\n");
        devolverSemaforo(mtxPIDS);
        printf("DEVOLVÍ SEMÁFORO\n");
	}

/**********************************************************************************************************************/
/****************************************************JUGADORES*********************************************************/
/**********************************************************************************************************************/

	void reiniciarJugador(t_jugador*jugador)
	{	
		jugador->saltando=0;
		jugador->incicioSalto=0;
		jugador->alturaSaltada=0;
		jugador->avanceHorizontal=0;
		jugador->enEscalera=-1;
		jugador->cayendo=0;
		jugador->martillo=0;
		jugador->evento=SIN_CAMBIOS;
		if(NRO_JUGADOR==0) 
		{
			jugador->x=10;
			jugador->estado=MIRANDO_DER;
		}
		else
		if(NRO_JUGADOR==1)
		{
			jugador->x=WIDTH - marioSprite2.getw()-10;
			jugador->estado=MIRANDO_IZQ;
		}
		jugador->y=HEIGHT-marioSprite1.geth()-ALTO_VIGA*2+6;
		actualizarAnimaciones(jugador);
		jugador->activo=1;
	}

	/**********************************************************************************************************************/

	void moverDerecha(t_jugador *jugador)
	{
		if(jugador->estado!=DERECHA)
		{
			jugador->estado=DERECHA;
			jugador->frameInicial=MIN_DER;
			jugador->frameMax=MAX_DER;
			jugador->frameActual=QUIETO_DER+1;
			jugador->avanceHorizontal=0;
		}
		else
		{
			jugador->avanceHorizontal++;
			jugador->frameActual++;	//Avanzamos al siguiente frame
			if(jugador->frameActual>jugador->frameMax)	//Si llegamos al ultimo frame de la animación
		 		jugador->frameActual=jugador->frameInicial;	//Volvemos al primer frame de la animación
		}
		jugador->x+=VELOCIDAD;
		corregirAlturaDer(jugador);
	}

	/**********************************************************************************************************************/

	void moverIzquierda(t_jugador *jugador)
	{
		if(jugador->estado!=IZQUIERDA)
		{
			jugador->estado=IZQUIERDA;
			jugador->frameInicial=MIN_IZQ;
			jugador->frameMax=MAX_IZQ;
			jugador->frameActual=QUIETO_IZQ+1;
			jugador->avanceHorizontal=0;
		}
		else
		{
			jugador->avanceHorizontal++;
			jugador->frameActual++;	//Avanzamos al siguiente frame
			if(jugador->frameActual>jugador->frameMax)	//Si llegamos al ultimo frame de la animación
		 		jugador->frameActual=jugador->frameInicial;	//Volvemos al primer frame de la animación
		}
		jugador->x-=VELOCIDAD;
		corregirAlturaIzq(jugador);
	}

	/**********************************************************************************************************************/

	void saltar(t_jugador*jugador)
	{
		//Si el jugador está saltando
			if(jugador->saltando==1)
			{
				if(jugador->estado==SALTO_MIRANDO_DER)
				{
					if(jugador->alturaSaltada==0)
					{
						jugador->frameActual=MIN_SALTO_DER;
					}
					if(jugador->alturaSaltada<ALTURA_SALTO)
					{
						jugador->y-=VELOCIDAD_SALTO;
						jugador->alturaSaltada+=VELOCIDAD_SALTO;
						jugador->x+=jugador->avanceHorizontal;
						if(jugador->x+22>WIDTH)
							jugador->x-=jugador->avanceHorizontal;
					}
					else
						if(jugador->alturaSaltada>=ALTURA_SALTO && jugador->alturaSaltada<(ALTURA_SALTO*2))
						{
							jugador->y+=VELOCIDAD_SALTO;
							jugador->alturaSaltada+=VELOCIDAD_SALTO;
							jugador->x+=jugador->avanceHorizontal;
							if(jugador->x+22>WIDTH)
								jugador->x-=jugador->avanceHorizontal;
						}
						else
						{
							jugador->frameActual=QUIETO_DER;
							jugador->saltando=0;
							jugador->estado=MIRANDO_DER;
							jugador->y=jugador->incicioSalto;
							actualizarAnimaciones(jugador);
							corregirAlturaDer(jugador);
						}
				}
				else
				if(jugador->estado==SALTO_MIRANDO_IZQ)
				{
					if(jugador->alturaSaltada==0)
					{
						jugador->frameActual=MIN_SALTO_IZQ;
					}
					if(jugador->alturaSaltada<ALTURA_SALTO)
					{
						jugador->y-=VELOCIDAD_SALTO;
						jugador->alturaSaltada+=VELOCIDAD_SALTO;
						jugador->x+=jugador->avanceHorizontal;
						if(jugador->x<0)
								jugador->x-=jugador->avanceHorizontal;
					}
					else
						if(jugador->alturaSaltada>=ALTURA_SALTO && jugador->alturaSaltada<(ALTURA_SALTO*2))
						{
							jugador->y+=VELOCIDAD_SALTO;
							jugador->alturaSaltada+=VELOCIDAD_SALTO;
							jugador->x+=jugador->avanceHorizontal;
							if(jugador->x<0)
								jugador->x-=jugador->avanceHorizontal;
						}
						else
						{
							jugador->frameActual=QUIETO_IZQ;
							jugador->saltando=0;
							jugador->estado=MIRANDO_IZQ;
							jugador->y=jugador->incicioSalto;
							actualizarAnimaciones(jugador);
							corregirAlturaIzq(jugador);
						}
				}
			}
	}

	/**********************************************************************************************************************/

	void caer(t_jugador*jugador)
	{
		int i=0;
		int marca=0;
		if(jugador->cayendo==0)
			return;
		while(i<cantVigas)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			//JUGADOR 1
			if(jugador->nroJugador==0)
			{
				if(vigaSprite.colision(marioSprite1)==TRUE)
				{
					jugador->cayendo=0;
					if((jugador->estado==IZQUIERDA || jugador->estado==SALTO_MIRANDO_IZQ || jugador->estado==QUIETO_IZQ))
					{
						corregirAlturaDer(jugador);
					}
					else
					if((jugador->estado==DERECHA || jugador->estado==SALTO_MIRANDO_DER || jugador->estado==QUIETO_DER))
					{
						corregirAlturaIzq(jugador);
					}
					actualizarAnimaciones(jugador);
					return;
				}
			}
			//JUGADOR 2
			else
			{
				if(jugador->nroJugador==1)
				{
					if(vigaSprite.colision(marioSprite2)==TRUE)  //marioSprite2 <---
					{
						jugador->cayendo=0;
						if((jugador->estado==IZQUIERDA || jugador->estado==SALTO_MIRANDO_IZQ || jugador->estado==QUIETO_IZQ))
						{
							corregirAlturaDer(jugador);
						}
						else
						if((jugador->estado==DERECHA || jugador->estado==SALTO_MIRANDO_DER || jugador->estado==QUIETO_DER))
						{
							corregirAlturaIzq(jugador);
						}
						actualizarAnimaciones(jugador);
						return;
					}
				}
			}
			i++;
		}
		//Si está en el aire	
		if((jugador->estado==IZQUIERDA || jugador->estado==SALTO_MIRANDO_IZQ || jugador->estado==MIRANDO_IZQ))
		{
			jugador->frameActual=MIN_SALTO_IZQ;
		}
		else
		if((jugador->estado==DERECHA || jugador->estado==SALTO_MIRANDO_DER || jugador->estado==MIRANDO_DER))
		{
			jugador->frameActual=MIN_SALTO_DER;
		}
		jugador->y+=VELOCIDAD_SALTO+1;
	}

	/**********************************************************************************************************************/

	int verificarEscaleras(t_jugador* jugador)
	{
		for(int i=0;i<CANT_ESCALERAS;i++)
		{
			if(NRO_JUGADOR==0)
			{
				if(escaleraVec[i].superpuesto(marioSprite1)==TRUE)
					return i;	//Devuelvo la escalera sobre la que se encuentra mario
			}
			else 
			if(NRO_JUGADOR==1)
			{
				if(escaleraVec[i].superpuesto(marioSprite2)==TRUE)
					return i;	//Devuelvo la escalera sobre la que se encuentra mario
			}
		}
		return NO_ESCALERA;
	}

	/**********************************************************************************************************************/

	void subirEscalera(t_jugador*jugador)
	{
		jugador->enEscalera=verificarEscaleras(jugador);
		if(jugador->enEscalera==NO_ESCALERA)
			return;
		if(jugador->estado!=EN_ESCALERA)
		{
			jugador->estado=EN_ESCALERA;
			actualizarAnimaciones(jugador);
			jugador->maxEscalera=escaleraVec[jugador->enEscalera].gety();
			jugador->minEscalera=escaleraVec[jugador->enEscalera].gety()+escaleraVec[jugador->enEscalera].geth();
		}
		jugador->y-=VELOCIDAD_SALTO;
		jugador->frameActual++;	//Avanzamos al siguiente frame
		if(jugador->frameActual>jugador->frameMax)	//Si llegamos al ultimo frame de la animación
		 	jugador->frameActual=jugador->frameInicial;	//Volvemos al primer frame de la animación
		
		if(NRO_JUGADOR==0)
		{
			if(marioSprite1.gety()+marioSprite1.geth()<=jugador->maxEscalera+4)
			{
				jugador->y=jugador->maxEscalera-marioSprite1.geth();
				jugador->frameActual=QUIETO_DER;
				jugador->estado=MIRANDO_DER;
				actualizarAnimaciones(jugador);
			}
		}
		else
		if(NRO_JUGADOR==1)
		{
			if(marioSprite2.gety()+marioSprite2.geth()<=jugador->maxEscalera+4)
			{
				jugador->y=jugador->maxEscalera-marioSprite2.geth();
				jugador->frameActual=QUIETO_DER;
				jugador->estado=MIRANDO_DER;
				actualizarAnimaciones(jugador);
			}
		}
	}

	/**********************************************************************************************************************/

	void bajarEscalera(t_jugador*jugador)
	{
		jugador->enEscalera=verificarEscaleras(jugador);
		if(jugador->enEscalera==NO_ESCALERA)
			return;
		if(jugador->estado!=EN_ESCALERA)
		{
			jugador->estado=EN_ESCALERA;
			actualizarAnimaciones(jugador);
			jugador->maxEscalera=escaleraVec[jugador->enEscalera].gety();
			jugador->minEscalera=escaleraVec[jugador->enEscalera].gety()+escaleraVec[jugador->enEscalera].geth();
		}
		jugador->y+=VELOCIDAD_SALTO;
		jugador->frameActual++;	//Avanzamos al siguiente frame
		if(jugador->frameActual>jugador->frameMax)	//Si llegamos al ultimo frame de la animación
		 	jugador->frameActual=jugador->frameInicial;	//Volvemos al primer frame de la animación
		if(NRO_JUGADOR==0)
		{
			if(marioSprite1.gety()+marioSprite1.geth()>=jugador->minEscalera)
			{
				jugador->y=jugador->minEscalera-marioSprite1.geth();
				jugador->frameActual=QUIETO_DER;
				jugador->estado=MIRANDO_DER;
				actualizarAnimaciones(jugador);
			}
		}
		else
		if(NRO_JUGADOR==1)
		{
			if(marioSprite2.gety()+marioSprite2.geth()>=jugador->minEscalera)
			{
				jugador->y=jugador->minEscalera-marioSprite2.geth();
				jugador->frameActual=QUIETO_DER;
				jugador->estado=MIRANDO_DER;
				actualizarAnimaciones(jugador);
			}
		}
	}
	/**********************************************************************************************************************/

	void actualizarAnimaciones(t_jugador *jugador)
	{
		switch(jugador->estado)
		{
			case MIRANDO_DER:
							jugador->frameActual=QUIETO_DER;
							jugador->frameInicial=QUIETO_DER;
							jugador->frameMax=QUIETO_DER;
							break;
			case DERECHA:
							jugador->frameInicial=MIN_DER+1;
							jugador->frameMax=MAX_DER;
							jugador->frameActual=QUIETO_DER;
							break;
			case IZQUIERDA:
							jugador->frameInicial=MIN_IZQ+1;
							jugador->frameMax=MAX_IZQ;
							jugador->frameActual=QUIETO_IZQ;
							break;
			case MIRANDO_IZQ:
							jugador->frameActual=QUIETO_IZQ;
							jugador->frameInicial=QUIETO_IZQ;
							jugador->frameMax=QUIETO_IZQ;
							break;
			case SALTO_MIRANDO_DER:
							jugador->frameActual=MIN_SALTO_DER;
							jugador->frameInicial=MIN_SALTO_DER;
							jugador->frameMax=MAX_SALTO_DER;
			case SALTO_MIRANDO_IZQ:
							jugador->frameActual=MIN_SALTO_IZQ;
							jugador->frameInicial=MIN_SALTO_IZQ;
							jugador->frameMax=MAX_SALTO_IZQ;
			case EN_ESCALERA:
							jugador->frameActual=MIN_ESCALERA;
							jugador->frameInicial=MIN_ESCALERA;
							jugador->frameMax=MAX_ESCALERA;
							break;
			case MUERTO:	
							jugador->frameActual=MIN_MUERTO;
							jugador->frameInicial=MIN_MUERTO;
							jugador->frameMax=MAX_MUERTO;
							break;
			default:
							break;
		}
	}

	/**********************************************************************************************************************/

	void corregirAlturaDer(t_jugador*jugador)
	{
		int marca=0;
		int i=0;
		int y;
		jugador->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			//SI ES EL JUGADOR 1
			if(NRO_JUGADOR==0)
			{
				y=vigaSprite.sobreDer(marioSprite1,3);
				if(y!=-1)
					marca++;
				if(y==1)
				{
					marca++;
					jugador->y=vigasVec[i].y+3-marioSprite1.geth();
				}
				if(y==2)
				{			
					jugador->y=vigasVec[i].y+3-marioSprite1.geth();
					marca++;
				}
			}
			//SI ES EL JUGADOR 2
			else
			if(NRO_JUGADOR==1)
			{
				y=vigaSprite.sobreDer(marioSprite2,3);
				if(y!=-1)
					marca++;
				if(y==1)
				{
					marca++;
					jugador->y=vigasVec[i].y+3-marioSprite2.geth();
				}
				if(y==2)
				{			
					jugador->y=vigasVec[i].y+3-marioSprite2.geth();
					marca++;
				}
			}

			i++;
		 }
		 //Si está en el aire
		 if(i>=cantVigas && y==-1)
		 {
		 	//jugador->x-=VELOCIDAD;
		 	jugador->cayendo=1;
		 }
	}

	/**********************************************************************************************************************/

	void corregirAlturaIzq(t_jugador*jugador)
	{
		int marca=0;
		int i=0;
		int y;
		jugador->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			if(jugador->nroJugador==0)
			{
				y=vigaSprite.sobreIzq(marioSprite1,3);
				if(y!=-1)
					marca++;
				if(y==1)
				{
					marca++;
					jugador->y=vigasVec[i].y+3-marioSprite1.geth();
				}
				if(y==2)
				{			
					jugador->y=vigasVec[i].y+3-marioSprite1.geth();
					marca++;
				}
			}
			//SI ES EL JUGADOR 2
			else
			if(jugador->nroJugador==1)
			{
				y=vigaSprite.sobreIzq(marioSprite2,3);
				if(y!=-1)
					marca++;
				if(y==1)
				{
					marca++;
					jugador->y=vigasVec[i].y+3-marioSprite2.geth();
				}
				if(y==2)
				{			
					jugador->y=vigasVec[i].y+3-marioSprite2.geth();
					marca++;
				}
			}
			i++;
		}
		 //Si está en el aire
		 if(i>=cantVigas && y==-1)
		 {
		 	//jugador->x+=VELOCIDAD;
		 	jugador->cayendo=1;
		 }
	}

/**********************************************************************************************************************/
/****************************************************BARRILES**********************************************************/
/**********************************************************************************************************************/

	int crearBarril(int*recorrido)
	{
	    for(int i=0;i<MAX_BARRILES;i++)
	    {         
	    	if(barriles[i].activo==0)
	    	{   
	    		if(ORIGEN_BARRIL%2!=0)     	//ALTERNA EL LADO DE ORIGEN DE LOS BARRILES
	    		{
	    			barriles[i].x=pilaBarriles1.getw()-5+monoSpriteDer.geth()/2;
	    			barriles[i].sentido=1;
	    		}
	        	else
	        	{
	        		barriles[i].x=WIDTH-pilaBarriles2.getw()+5-monoSpriteIzq.geth()/2;
	        		barriles[i].sentido=-1;
	        	}
	        	barriles[i].y=142+monoSpriteDer.geth();
	        	barrilesVec[i].setx(barriles[i].x);
	        	barrilesVec[i].sety(barriles[i].y);
	        	//GENERAMOS EL RECORRIDO DEL BARRIL
	        	for (int x=0;x<CANT_ESCALERAS_ROTAS;x++)
	        		barriles[i].recorrido[x]=recorrido[x];
	        	//ACTIVAMOS EL BARRIL
	        	barriles[i].activo=1;
	        	ORIGEN_BARRIL++;
	        	return 1;        
	    	}     
	    }          
	    return -1;
	}

	/**********************************************************************************************************************/

	void dibujarBarriles()
	{
		for (int i=0;i<MAX_BARRILES;i++)
		{
			if(barriles[i].activo==1)
			{
				//Avanzamos en el sprite, cambiamos de frame
				barriles[i].frameActual+=1*barriles[i].sentido;
				if(barriles[i].frameActual>barriles[i].frameMax)
					barriles[i].frameActual=barriles[i].frameInicial;
				if(barriles[i].frameActual<barriles[i].frameInicial)
					barriles[i].frameActual=barriles[i].frameMax;
				barrilesVec[i].setx(barriles[i].x);//SOLO PARA VERIFICAR COLISIONES
				barrilesVec[i].sety(barriles[i].y);//SOLO PARA VERIFICAR COLISIONES
				barrilSprite.setx(barriles[i].x);
				barrilSprite.sety(barriles[i].y);
				barrilSprite.selframe(barriles[i].frameActual);
				barrilSprite.dibujar(pantalla);
			}
		}
	}

	/**********************************************************************************************************************/

	void moverBarriles()
	{
		int x=0;
		int marca=0;
		for(int i=0;i<MAX_BARRILES;i++)
	    {         
	    	if(barriles[i].activo==1)
	    	{   
	    		//SI EL BARRIL ESTÁ CAYENDO
				if(barriles[i].cayendo==1)
				{
					marca=0;
					x=0;
					while(x<cantVigas&&marca==0)
					{
						vigaSprite.setx(vigasVec[x].x);	
						vigaSprite.sety(vigasVec[x].y);
						if(vigaSprite.colision(barrilesVec[i])==TRUE)
						{
							barriles[i].cayendo=0;
							if(barriles[i].sentido==-1)
							{
								corregirAlturaDerBarril(&barriles[i]);
							}
							else
							if(barriles[i].sentido==1)
							{
								corregirAlturaIzqBarril(&barriles[i]);
							}
							marca++;
						}
						x++;
					}
					//Si está en el aire (PARA CAMBIAR LAS ANIMACIONES)
					// if((jugador->estado==IZQUIERDA || jugador->estado==SALTO_MIRANDO_IZQ || jugador->estado==MIRANDO_IZQ))
					// {
					// 	jugador->frameActual=MIN_SALTO_IZQ;
					// }
					// else
					// if((jugador->estado==DERECHA || jugador->estado==SALTO_MIRANDO_DER || jugador->estado==MIRANDO_DER))
					// {
					// 	jugador->frameActual=MIN_SALTO_DER;
					// }
					barriles[i].y+=VELOCIDAD_CAIDA_BARRIL;
					barrilesVec[i].sety(barriles[i].y);
				}
				//SI NO ESTÁ CAYENDO Y NO ESTÁ EN UNA ESCALERA
				if(barriles[i].estado!=EN_ESCALERA)
				{
		    		barriles[i].x+=VELOCIDAD_BARRIL*barriles[i].sentido;
		    		barrilesVec[i].setx(barriles[i].x);
		    		barrilesVec[i].sety(barriles[i].y);
		    		if(barriles[i].sentido==1)
		    			corregirAlturaDerBarril(&barriles[i]);
		    		if(barriles[i].sentido==-1)
		    				corregirAlturaIzqBarril(&barriles[i]);
		    		if(barriles[i].x+barrilSprite.getw()>WIDTH)
		    			barriles[i].sentido*=-1;
		    		else 
		    			if(barriles[i].x<0)
		    				barriles[i].sentido*=-1;
		    	}
		    	bajarEscaleraBarril(&barriles[i]);
	    	}     
	    }          
	}

	/**********************************************************************************************************************/

	int verificarEscalerasBarril(t_barril* barril)
	{
		for(int i=0;i<CANT_ESCALERAS_ROTAS;i++)
		{
			if(escaleraRotaVec[i].superpuesto(barrilesVec[barril->nroBarril])==TRUE)
					return i;	//Devuelvo la escalera sobre la que se encuentra el barril
		}
		return NO_ESCALERA;
	}


	/**********************************************************************************************************************/

	void bajarEscaleraBarril(t_barril* barril)
	{
		barril->enEscalera=verificarEscalerasBarril(barril);
		if(barril->enEscalera==NO_ESCALERA)
			return;
		if(barril->estado!=EN_ESCALERA)
		{
			if((barril->y>escaleraRotaVec[barril->enEscalera].gety() && barril->y+barrilesVec[barril->nroBarril].geth()<=escaleraRotaVec[barril->enEscalera].gety()+escaleraRotaVec[barril->enEscalera].geth()+3)
				||barril->y+barrilesVec[barril->nroBarril].geth()<escaleraRotaVec[barril->enEscalera].gety())
				return;
			//DECIDIMOS SI BAJAR O NO POR LA ESCALERA
			if(barril->recorrido[barril->enEscalera]==0)
				return;
			barril->estado=EN_ESCALERA;
			//FALTA COLOCOCAR LA ANIMACION (frameMax, frameInicial, frame actual)
			barril->minEscalera=escaleraRotaVec[barril->enEscalera].gety()+escaleraRotaVec[barril->enEscalera].geth();
		}
		barril->y+=VELOCIDAD_SALTO;
		barrilesVec[barril->nroBarril].sety(barril->y);
		// barril->frameActual++;	//Avanzamos al siguiente frame
		// if(barril->frameActual>barril->frameMax)	//Si llegamos al ultimo frame de la animación
		//  	barril->frameActual=barril->frameInicial;	//Volvemos al primer frame de la animación
		if(barril->y+barrilSprite.geth()>=barril->minEscalera)
		{
			barril->y=barril->minEscalera-barrilSprite.geth();
			barril->frameActual=0;
			barril->sentido*=-1;
			barril->frameInicial=0;
			barril->frameMax=7;
			barril->estado=NO_ESCALERA;
			barril->enEscalera=NO_ESCALERA;
		}
	}

	/**********************************************************************************************************************/

	void corregirAlturaDerBarril(t_barril*barril)
	{
		int marca=0;
		int i=0;
		int y;
		//barril->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			y=vigaSprite.sobreDer(barrilesVec[barril->nroBarril],3);
			if(y!=-1)
				marca++;
			// if(y==0)
			// {
			// 	printf("Sin cambios\n");
			// }
			if(y==1)
			{
				marca++;
				barril->y=vigasVec[i].y+3-barrilesVec[barril->nroBarril].geth();
			}
			if(y==2)
			{			
				barril->y=vigasVec[i].y+3-barrilesVec[barril->nroBarril].geth();
				marca++;
			}
			i++;
		 }
		 //Si está en el aire
		 if(i>=cantVigas && y==-1)
		 {
		 	barril->cayendo=1;
		 	//barril->sentido*=-1;
		 }
	}

	/**********************************************************************************************************************/

	void corregirAlturaIzqBarril(t_barril*barril)
	{
		int marca=0;
		int i=0;
		int y;
		//barril->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			y=vigaSprite.sobreIzq(barrilesVec[barril->nroBarril],3);
			if(y!=-1)
				marca++;
			// if(y==0)
			// {
			// 	printf("Sin cambios\n");
			// }
			if(y==1)
			{
				marca++;
				barril->y=vigasVec[i].y+3-barrilesVec[barril->nroBarril].geth();
			}
			if(y==2)
			{			
				barril->y=vigasVec[i].y+3-barrilesVec[barril->nroBarril].geth();
				marca++;
			}
			i++;
		 }
		 //Si está en el aire
		 if(i>=cantVigas && y==-1)
		 {
		 	barril->cayendo=1;
		 	//barril->sentido*=-1;
		 }
	}

/**********************************************************************************************************************/
/*****************************************************LLAMAS***********************************************************/
/**********************************************************************************************************************/

	int crearLlama()
	{
	    for(int i=0;i<MAX_LLAMAS;i++)
	    {         
	    	if(llamas[i].activo==0)
	    	{   
	    		llamas[i].x=WIDTH/2-llamaSprite.getw()/2;
				llamas[i].y=HEIGHT - vigaSprite.geth()-llamaSprite.geth();

				llamas[i].sentido=movimientos.sentidoLlama;

	        	//Recorrido de la llama
	        	if(NRO_LLAMA>4)
	        		NRO_LLAMA=0;

	        	if(NRO_LLAMA==0)
	        	{
	        		for(int x=0;x<5;x++)
	        			llamas[i].recorrido[x]=id_jugador.recorrerllama1[x];
	        	}
	        	if(NRO_LLAMA==1)
	        	{
	        		for(int x=0;x<5;x++)
	        			llamas[i].recorrido[x]=id_jugador.recorrerllama2[x];
	        	}
	        	if(NRO_LLAMA==2)
	        	{
	        		for(int x=0;x<5;x++)
	        			llamas[i].recorrido[x]=id_jugador.recorrerllama3[x];
	        	}
	        	if(NRO_LLAMA==3)
	        	{
	        		for(int x=0;x<5;x++)
	        			llamas[i].recorrido[x]=id_jugador.recorrerllama4[x];
	        	}
	        	if(NRO_LLAMA==4)
	        	{
	        		for(int x=0;x<5;x++)
	        			llamas[i].recorrido[x]=id_jugador.recorrerllama5[x];
	        	}

	        	NRO_LLAMA++;
	        	llamasVec[i].setx(llamas[i].x);
	        	llamasVec[i].sety(llamas[i].y);
	        	llamas[i].activo=1;
	        	CREAR_LLAMA=0;
	        	printf("Llama creada!\n");
	        	return 1;        
	    	}     
	    }          
	    return -1;
	}

	/**********************************************************************************************************************/

	void moverLlamas()
	{
		int x=0;
		int marca=0;
		for(int i=0;i<MAX_LLAMAS;i++)
	    {        
	    	if(llamas[i].activo==1)
	    	{   
				//SI NO ESTÁ EN UNA ESCALERA
				if(llamas[i].estado!=SUBIENDO_ESCALERA && llamas[i].estado!=BAJANDO_ESCALERA)
				{
		    		llamas[i].x+=VELOCIDAD_HORIZONTAL_LLAMA*llamas[i].sentido;
		    		llamasVec[i].setx(llamas[i].x);
		    		llamasVec[i].sety(llamas[i].y);
		    		if(llamas[i].sentido==1)
		    			corregirAlturaDerLlama(&llamas[i]);
		    		if(llamas[i].sentido==-1)
		    				corregirAlturaIzqLlama(&llamas[i]);
		    		if(llamas[i].x+llamaSprite.getw()>WIDTH)
		    			llamas[i].sentido=-1;
		    		else 
		    			if(llamas[i].x<0)
		    				llamas[i].sentido=1;
		    	}
		    	usarEscaleraRotaLlama(&llamas[i]);
		    	usarEscaleraLlama(&llamas[i]);
	    	}     
	    }          
	}

	/**********************************************************************************************************************/

	void dibujarLlamas()
	{
		for (int i=0;i<MAX_LLAMAS;i++)
		{
			if(llamas[i].activo==1)
			{
				//Avanzamos en el sprite, cambiamos de frame
				llamas[i].frameActual+=1*llamas[i].sentido;
				if(llamas[i].frameActual>llamas[i].frameMax)
					llamas[i].frameActual=llamas[i].frameInicial;
				if(llamas[i].frameActual<llamas[i].frameInicial)
					llamas[i].frameActual=llamas[i].frameMax;
				llamasVec[i].setx(llamas[i].x);//SOLO PARA VERIFICAR COLISIONES
				llamasVec[i].sety(llamas[i].y);//SOLO PARA VERIFICAR COLISIONES
				llamaSprite.setx(llamas[i].x);
				llamaSprite.sety(llamas[i].y);
				llamaSprite.selframe(llamas[i].frameActual);
				llamaSprite.dibujar(pantalla);
				if(llamas[i].cambiarAnimacion==3)
			    {
			    	llamas[i].cambiarAnimacion=0;
			  	llamas[i].frameActual+=llamas[i].sentido;
			  	if(llamas[i].frameActual>llamas[i].frameMax)
			   		llamas[i].frameActual=llamas[i].frameInicial;
			   	if(llamas[i].frameActual<llamas[i].frameInicial)
			   		llamas[i].frameActual=llamas[i].frameMax;
			 	}
			 	else
			  		llamas[i].cambiarAnimacion++;
			}
		}
	}

	/**********************************************************************************************************************/

	void corregirAlturaDerLlama(t_llama* llama)
	{
		int marca=0;
		int i=0;
		int y;
		//barril->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			y=vigaSprite.sobreDer(llamasVec[llama->nroLlama],3);
			if(y!=-1)
				marca++;
			// if(y==0)
			// {
			// 	printf("Sin cambios\n");
			// }
			if(y==1)
			{
				marca++;
				llama->y=vigasVec[i].y+3-llamasVec[llama->nroLlama].geth();
			}
			if(y==2)
			{			
				llama->y=vigasVec[i].y+3-llamasVec[llama->nroLlama].geth();
				marca++;
			}
			i++;
		 }
		 //Si está en el aire
		if(i>=cantVigas && y==-1)
		{
		 	llama->sentido*=-1;
		}
	}

	/**********************************************************************************************************************/

	void corregirAlturaIzqLlama(t_llama* llama)
	{
		int marca=0;
		int i=0;
		int y;
		//barril->cayendo=0;
		while(i<cantVigas&&marca==0)
		{
			vigaSprite.setx(vigasVec[i].x);	
			vigaSprite.sety(vigasVec[i].y);
			y=vigaSprite.sobreIzq(llamasVec[llama->nroLlama],3);
			if(y!=-1)
				marca++;
			// if(y==0)
			// {
			// 	printf("Sin cambios\n");
			// }
			if(y==1)
			{
				marca++;
				llama->y=vigasVec[i].y+3-llamasVec[llama->nroLlama].geth();
			}
			if(y==2)
			{			
				llama->y=vigasVec[i].y+3-llamasVec[llama->nroLlama].geth();
				marca++;
			}
			i++;
		 }
		 //Si está en el aire
		if(i>=cantVigas && y==-1)
		{
		 	llama->sentido*=-1;
		}
	}

	/**********************************************************************************************************************/

	int verificarEscalerasRotasLlama(t_llama* llama)
	{
		for(int i=0;i<CANT_ESCALERAS_ROTAS-2;i++)	//El -2 evita que las llamas suban al ultimo nivel
		{
			//if(llamasVec[llama->nroLlama].superpuesto(escaleraRotaVec[i])==TRUE)
			if(escaleraRotaVec[i].superpuesto(llamasVec[llama->nroLlama])==TRUE)
					return i;	//Devuelvo la escalera sobre la que se encuentra la llama
		}
		return NO_ESCALERA;
	}

	/**********************************************************************************************************************/

	int verificarEscalerasLlama(t_llama* llama)
	{
		for(int i=0;i<CANT_ESCALERAS-4;i++)		//El -4 evita que las llamas suban al ultimo nivel
		{
			if(escaleraVec[i].superpuesto(llamasVec[llama->nroLlama])==TRUE)
					return i;	//Devuelvo la escalera sobre la que se encuentra la llama
		}
		return NO_ESCALERA;
	}

	/**********************************************************************************************************************/

	void usarEscaleraRotaLlama(t_llama* llama)
	{
		llama->enEscalera=verificarEscalerasRotasLlama(llama);
		if(llama->enEscalera==NO_ESCALERA)
			return;
		if(llama->estado!=SUBIENDO_ESCALERA && llama->estado!=BAJANDO_ESCALERA)
		{
			//DECIDIMOS SI USAR O NO LA ESCALERA
			if(llama->recorrido[llama->posActual]==0)
			{
				llama->posActual++;
				if(llama->posActual>4)
					llama->posActual=0;
				return;
			}
			llama->posActual++;
			if(llama->posActual>4)
				llama->posActual=0;
			if((llama->y>escaleraRotaVec[llama->enEscalera].gety() && llama->y+llamasVec[llama->nroLlama].geth()<=escaleraRotaVec[llama->enEscalera].gety()+escaleraRotaVec[llama->enEscalera].geth()+3)
				||llama->y+llamasVec[llama->nroLlama].geth()<escaleraRotaVec[llama->enEscalera].gety())
			{
				llama->estado=SUBIENDO_ESCALERA;
				llama->maxEscalera=escaleraRotaVec[llama->enEscalera].gety();
			}
			else
			{
				llama->estado=BAJANDO_ESCALERA;
				llama->minEscalera=escaleraRotaVec[llama->enEscalera].gety()+escaleraRotaVec[llama->enEscalera].geth();
			}
		}
		if(llama->estado==BAJANDO_ESCALERA)
		{
			llama->y+=VELOCIDAD_ESCALERA_LLAMA;
			llamasVec[llama->nroLlama].sety(llama->y);
			if(llama->y+llamaSprite.geth()>=llama->minEscalera)
			{
				llama->y=llama->minEscalera-llamaSprite.geth();
				llama->frameActual=0;
				if(llama->posActual%2==0)
					llama->sentido*=-1;
				llama->frameInicial=0;
				llama->frameMax=3;
				llama->estado=NO_ESCALERA;
				llama->enEscalera=NO_ESCALERA;
			}
		}
		if(llama->estado==SUBIENDO_ESCALERA)
		{
			llama->y-=VELOCIDAD_ESCALERA_LLAMA;
			llamasVec[llama->nroLlama].sety(llama->y);
			// barril->frameActual++;	//Avanzamos al siguiente frame
			// if(barril->frameActual>barril->frameMax)	//Si llegamos al ultimo frame de la animación
			//  	barril->frameActual=barril->frameInicial;	//Volvemos al primer frame de la animación
			if(llama->y+llamaSprite.geth()<=llama->maxEscalera+3)
			{
				llama->y=llama->maxEscalera-llamaSprite.geth();
				llama->frameActual=0;
				if(llama->posActual%2==1)
					llama->sentido*=-1;
				llama->frameInicial=0;
				llama->frameMax=3;
				llama->estado=NO_ESCALERA;
				llama->enEscalera=NO_ESCALERA;
			}
		}
	}

	/**********************************************************************************************************************/

	void usarEscaleraLlama(t_llama* llama)
	{
		llama->enEscalera=verificarEscalerasLlama(llama);
		if(llama->enEscalera==NO_ESCALERA)
			return;
		if(llama->estado!=SUBIENDO_ESCALERA && llama->estado!=BAJANDO_ESCALERA)
		{
			//DECIDIMOS SI USAR O NO LA ESCALERA
			if(llama->recorrido[llama->posActual]==0)
			{
				llama->posActual++;
				if(llama->posActual>5)
					llama->posActual=0;
				return;
			}
			llama->posActual++;
			if(llama->posActual>4)
				llama->posActual=0;
			if((llama->y>escaleraVec[llama->enEscalera].gety() && llama->y+llamasVec[llama->nroLlama].geth()<=escaleraVec[llama->enEscalera].gety()+escaleraVec[llama->enEscalera].geth()+3)
				||llama->y+llamasVec[llama->nroLlama].geth()<escaleraVec[llama->enEscalera].gety())
			{
				llama->estado=SUBIENDO_ESCALERA;
				llama->maxEscalera=escaleraVec[llama->enEscalera].gety();
			}
			else
			{
				llama->estado=BAJANDO_ESCALERA;
				llama->minEscalera=escaleraVec[llama->enEscalera].gety()+escaleraVec[llama->enEscalera].geth();
			}
		}
		if(llama->estado==BAJANDO_ESCALERA)
		{
			llama->y+=VELOCIDAD_ESCALERA_LLAMA;
			llamasVec[llama->nroLlama].sety(llama->y);
			if(llama->y+llamaSprite.geth()>=llama->minEscalera)
			{
				llama->y=llama->minEscalera-llamaSprite.geth();
				llama->frameActual=0;
				if(llama->posActual%2==0)
					llama->sentido*=-1;
				llama->frameInicial=0;
				llama->frameMax=3;
				llama->estado=NO_ESCALERA;
				llama->enEscalera=NO_ESCALERA;
			}
		}
		if(llama->estado==SUBIENDO_ESCALERA)
		{
			llama->y-=VELOCIDAD_ESCALERA_LLAMA;
			llamasVec[llama->nroLlama].sety(llama->y);
			if(llama->y+llamaSprite.geth()<=llama->maxEscalera+3)
			{
				llama->y=llama->maxEscalera-llamaSprite.geth();
				llama->frameActual=0;
				if(llama->posActual%2==1)
					llama->sentido*=-1;
				llama->frameInicial=0;
				llama->frameMax=3;
				llama->estado=NO_ESCALERA;
				llama->enEscalera=NO_ESCALERA;
			}
		}
	}

/**********************************************************************************************************************/
/******************************************************VIGAS***********************************************************/
/**********************************************************************************************************************/

	void iniciarVigas()
	{
		int x=0, y=HEIGHT;
		int i=0, j=0;
		int y_incremento_nivel=DISTANCIA_ENTRE_PISOS;
		int decremento_y=3;
		y-=decremento_y*16;
		y-=2;
		cantVigas=0;
		//NIVEL 1
		for(i=0; i<9; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			y+=decremento_y;
			cantVigas++;
		}
		j=i;
		y-=decremento_y;
		for(i=j; i<j+9; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y-=decremento_y;
		}
		j=i;
		y-=y_incremento_nivel;
		x=0;
		x+=vigaSprite.getw();
		//NIVEL 2
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			y-=decremento_y;
			cantVigas++;
		}
		j=i;
		y+=decremento_y;
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y+=decremento_y;
		}
		j=i;
		y-=y_incremento_nivel;
		y-=decremento_y*21;
		x=0;
		//NIVEL 3
		y+=decremento_y;
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y+=decremento_y;
		}
		x+=vigaSprite.getw()*2;
		y-=decremento_y;
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y-=decremento_y;
		}
		y-=y_incremento_nivel;
		x=0;
		x+=vigaSprite.getw();
		//NIVEL 4
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			y-=decremento_y;
			cantVigas++;
		}
		j=i;
		y+=decremento_y;
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y+=decremento_y;
		}
		y-=decremento_y*21;
		x=0;
			j=i;
		y-=y_incremento_nivel;
		//NIVEL 5
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			y+=decremento_y;
			cantVigas++;
		}
		y-=decremento_y;
		x+=vigaSprite.getw()*2;
		for(i=j; i<j+8; i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
			y-=decremento_y;
		}
		y-=y_incremento_nivel;
		x=0;
		//NIVEL PAULINE
		x+=vigaSprite.getw();
		for(i=j; i<j+5; i++)
		{
			y+=1;
			x+=vigaSprite.getw();
		}
		for(i=0;i<6;i++)
		{
			vigasVec[cantVigas].x=x;
			vigasVec[cantVigas].y=y;
			x+=vigaSprite.getw();
			cantVigas++;
		}
	}

	/**********************************************************************************************************************/

	void dibujarVigas()
	{
		SDL_Rect area;
		for(int i=0;i<cantVigas;i++)
		{

			vigaSprite.setx(vigasVec[i].x);
			vigaSprite.sety(vigasVec[i].y);
			vigaSprite.selframe(0);
			vigaSprite.dibujar(pantalla);
		}
	}

/**********************************************************************************************************************/
/****************************************************PRINCESA**********************************************************/
/**********************************************************************************************************************/

	void moverPrincesa()
	{
		princesa.x+=1*princesa.sentido;
		princesa.cambiarAnimacion++;
	    if(princesa.x>500)
	    {
	    	princesa.sentido*=-1;
	    	princesa.frameInicial=4;
	    	princesa.frameMax=7;
	    	princesa.frameActual=princesa.frameInicial;
	    }
	    else 
	    	if(princesa.x<300)
	    	{
	    		princesa.sentido*=-1;
	    		princesa.frameInicial=0;
	    		princesa.frameMax=3;
	    		princesa.frameActual=princesa.frameInicial;
	    	}
	    if(princesa.cambiarAnimacion==3)
	    {
	    	princesa.cambiarAnimacion=0;
			princesa.frameActual++;
			if(princesa.frameActual>princesa.frameMax)
				princesa.frameActual=princesa.frameInicial;
		}
	}

/**********************************************************************************************************************/
/*****************************************************MONOS************************************************************/
/**********************************************************************************************************************/

	void crearMonos()
	{
		monoIzq.frameInicial=monoDer.frameInicial=0;
	    monoIzq.frameActual=monoDer.frameActual=0;
	    monoIzq.frameMax=monoDer.frameMax=29;
	    monoIzq.estado=monoDer.estado=0;
	    monoSpriteIzq.setx(pilaBarriles1.getw()-5);
	    monoSpriteIzq.sety(167);
	    monoSpriteIzq.selframe(monoIzq.frameInicial);
	    monoSpriteDer.setx(WIDTH-pilaBarriles2.getw()-monoSpriteDer.getw()+5);
	    monoSpriteDer.sety(167);
	    monoSpriteDer.selframe(monoDer.frameInicial);
	    //
	    monoIzqNada.setx(pilaBarriles1.getw()-5);
	    monoIzqNada.sety(167);
	    monoIzqNada.selframe(monoIzq.frameInicial);
	    monoDerNada.setx(WIDTH-pilaBarriles2.getw()-monoSpriteDer.getw()+5);
	    monoDerNada.sety(167);
	    monoDerNada.selframe(monoDer.frameInicial);
	    //
	    monoDer.tirandoBarril=0;
	    monoIzq.tirandoBarril=0;
	}

	/**********************************************************************************************************************/

	void actualizarMonos()
	{
		if(monoDer.tirandoBarril==0)
		{
			monoDer.estado++;
			monoDerNada.selframe(monoDer.frameActual);
			monoDerNada.dibujar(pantalla);
			if(monoDer.estado==1)
			{
				monoDer.estado=0;
				if(monoDer.frameActual>=monoDer.frameMax)
					monoDer.frameActual=monoDer.frameInicial;
				monoDer.frameActual++;
			}
		}
		//
		if(monoDer.tirandoBarril==1)
		{
			monoDer.estado++;
			monoSpriteDer.selframe(monoDer.frameActual);
			monoSpriteDer.dibujar(pantalla);
			if(monoDer.estado==3)
		    {
		    	monoDer.estado=0;
				monoDer.frameActual++;
				if(monoDer.frameActual==5)
					crearBarril(recorridoBarril);
				if(monoDer.frameActual>=monoDer.frameMax)
				{
					monoDer.tirandoBarril=0;
					monoDer.frameInicial=0;
					monoDer.frameActual=0;
					monoDer.frameMax=29;
					monoDer.estado=0;
				}
			}
		}
		//
		if(monoIzq.tirandoBarril==0)
		{
			monoIzq.estado++;
			monoIzqNada.selframe(monoIzq.frameActual);
			monoIzqNada.dibujar(pantalla);
			if(monoIzq.estado==1)
			{
				monoIzq.estado=0;
				if(monoIzq.frameActual>=monoIzq.frameMax)
					monoIzq.frameActual=monoIzq.frameInicial;
				monoIzq.frameActual++;
			}
		}
		//
		if(monoIzq.tirandoBarril==1)
		{
			monoIzq.estado++;
			monoSpriteIzq.selframe(monoIzq.frameActual);
			monoSpriteIzq.dibujar(pantalla);
			if(monoIzq.estado==3)
		    {
		    	monoIzq.estado=0;
				monoIzq.frameActual++;
				if(monoIzq.frameActual==5)
					crearBarril(recorridoBarril2);
				if(monoIzq.frameActual>=monoIzq.frameMax)
				{
					monoIzq.tirandoBarril=0;
					monoIzq.frameInicial=0;
					monoIzq.frameActual=0;
					monoIzq.frameMax=29;
					monoDer.estado=0;
				}
			}
		}
	}

/**********************************************************************************************************************/
/*********************************************MENSAJES EN PANTALLA*****************************************************/
/**********************************************************************************************************************/

	void cargarInicio()
	{
		cargarTexto("fonts/Jumpman.ttf",100,"DONKEY KONG", colorRojo, 150,100);	
		cargarImagen("img/donkey_inicial.bmp",300,200);
		cargarTexto("fonts/Jumpman.ttf",30, "Ingrese su nombre: ",colorBlanco,150,430);
		if(SDL_Flip(pantalla)==-1)
		{
			fprintf(stderr, "Error al realizar el flipping de la superficie %s\n",SDL_GetError());
			exit(1);	
		}
		captarTexto();	
	}

	/**********************************************************************************************************************/

	void captarTexto()
	{
		int finalizarCaptura=0;
		int tiempoFrame;
		SDL_Event evento, evento_ant;
		SDL_Rect borrar;
		int i=0;
		char nombre[10];
		borrar.x = 400;
		borrar.y= 430;
		borrar.w = 700;
		borrar.h = 200;
		int cant = 0;
		while(finalizarCaptura==0)
		{
			tiempoFrame = SDL_GetTicks();	//Tiempo de inicio del frame
			if(SDL_PollEvent(&evento))		//Verificamos si ocurre un evento en el juego
			{
				if ( evento.type == SDL_KEYDOWN)
				{
					if(evento.key.keysym.sym>=65 && evento.key.keysym.sym<=90 ||evento.key.keysym.sym>=97 && evento.key.keysym.sym<=122)
					{
						if(i<10)
						{
							nombre[i] = evento.key.keysym.sym;
							i++;
							nombre[i]='\0';
						}		
					}
					if(evento.key.keysym.sym==8)
					{ //backspace
						if(i>0)
						{
							nombre[--i]='\0';	
						}
					    SDL_FillRect(pantalla, &borrar, SDL_MapRGB(pantalla->format, 0, 0, 0));
					}
					if(i>0)
						cargarTexto("fonts/Jumpman.ttf",30, nombre,colorBlanco,400,430);
					SDL_Flip(pantalla);
					 if(evento.key.keysym.sym==13)
					 	break;
				}	
				if(evento.type==SDL_QUIT)
				{
					finalizarCaptura=1;
					finalizar=1;
				}
			}
			evento_ant.key.keysym.sym=evento.key.keysym.sym;
			tiempoFrame = SDL_GetTicks()-tiempoFrame;	//Tiempo de finalización del frame
			if(tiempoFrame < 1000/200)
    			SDL_Delay(Uint32(1000/200-tiempoFrame));
		}
		if(i>0)
		{
			toUpperString(nombre);
			strcpy(jugador[0].nombre,nombre);
			strcpy(jugador[1].nombre,nombre);
		}
		else
		{
			strcpy(jugador[0].nombre,"Mario\0");
			strcpy(jugador[1].nombre,"Mario\0");
		}
		cargarTexto("fonts/Jumpman.ttf",25,"Esperando rival...",colorRojo,150,500);
      	SDL_Flip(pantalla);
	}

	/**********************************************************************************************************************/

	void cargarImagen (const char * path, int x, int y)
	{
		SDL_Rect areaDestino;
		SDL_Surface * imagenDestino = SDL_LoadBMP(path);	
		if(imagenDestino == NULL)
		{
			fprintf(stderr, "No se puede cargar la imagen: %s\n", SDL_GetError());
		 	exit(1);
		}	
		SDL_SetColorKey(imagenDestino,SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(imagenDestino->format, 255, 0, 255));
		areaDestino.x = x;
		areaDestino.y = y;
		areaDestino.w = imagenDestino->w;
		areaDestino.h = imagenDestino->h;
		SDL_BlitSurface(imagenDestino, NULL,pantalla, &areaDestino);
		SDL_FreeSurface(imagenDestino); //Uno por cada superficie representada.
	}

	/**********************************************************************************************************************/

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

	/**********************************************************************************************************************/

	void escribirMensaje(const char* texto, TTF_Font* fuente, SDL_Color color_fuente, int x, int y)
	{
		SDL_Rect areaDestino;
		//Cargamos el texto sobre la superficie
		SDL_Surface *mensaje = TTF_RenderText_Solid(fuente, texto, color_fuente);	
		//Comprobamos si fue exitosa la carga del texto sobre la superficie
		if(mensaje == NULL) 
		{
			fprintf(stderr, "Error al renderizar la fuente sobre la superficie %s\n",TTF_GetError());
			exit(1);
		}
		areaDestino.x= x;
		areaDestino.y= y;
		areaDestino.w = mensaje->w;
		areaDestino.h = mensaje->h;	
		//Finalmente volcamos los cambios sobre la pantalla
		SDL_BlitSurface(mensaje,NULL,pantalla,&areaDestino);
		SDL_FreeSurface(mensaje);
	}

	/**********************************************************************************************************************/

	void escribirNombreEnPantalla(const t_jugador* jugador)
	{
		char descripcion[70]="Jugador: \0";
		strcat(descripcion,jugador->nombre);
		if(jugador->nroJugador==0)
			escribirMensaje(descripcion,fuenteJugador,colorBlanco,15,5);
		else
			escribirMensaje(descripcion,fuenteJugador,colorBlanco,WIDTH-(8*strlen(descripcion))-30,5);
	}

	/**********************************************************************************************************************/

	void cargarTexto(const char *path_fuente, int tam_fuente, const char * texto, SDL_Color color_fuente, int x, int y)
	{
		SDL_Rect areaDestino;
		TTF_Font *fuente; //almacena la fuente de los textos.
		SDL_Surface *mensaje;
		fuente = TTF_OpenFont(path_fuente, tam_fuente);  //cargo la fuente.
		//Comprobamos si la fuente fue cargada correctamente
		if(fuente==NULL)
		{
			fprintf(stderr, "Error al cargar la fuente: %s\n",TTF_GetError());
			exit(1);
		}
		//Cargamos el texto sobre la superficie
		mensaje = TTF_RenderText_Solid(fuente, texto, color_fuente);	
		//Comprobamos si fue exitosa la carga del texto sobre la superficie
		if(mensaje == NULL) 
		{
			fprintf(stderr, "Error al renderizar la fuente sobre la superficie %s\n",TTF_GetError());
			exit(1);
		}
		areaDestino.x= x;
		areaDestino.y= y;
		areaDestino.w = mensaje->w;
		areaDestino.h = mensaje->h;	
		//Finalmente volcamos los cambios sobre la pantalla
		SDL_BlitSurface(mensaje,NULL,pantalla,&areaDestino);
		SDL_FreeSurface(mensaje);
		TTF_CloseFont(fuente);
	}

	/**********************************************************************************************************************/

	void cargarRescateEnPantalla()
	{
		int x1=15;
		int x2=WIDTH-x1-30;
		int y=35;
		if(jugador[0].cantidadDeRescates==RESCATES)
		{
			jugador[0].cantidadDeRescates=jugador[1].cantidadDeRescates=0;
			//jugador[0].evento=GANO_LA_PARTIDA;
			cargarMensajeDeTriunfo(&jugador[0]);
			//finalizar=1;
			return ;
		}
		
		if(jugador[1].cantidadDeRescates==RESCATES)
		{
			jugador[0].cantidadDeRescates=jugador[1].cantidadDeRescates=0;
			//jugador[1].evento=GANO_LA_PARTIDA;
			cargarMensajeDeTriunfo(&jugador[1]);
			return ;
		}		
		for(int i=0; i<jugador[0].cantidadDeRescates; i++)
		{
			rescateIcono.setx(x1);
			rescateIcono.sety(y);
			rescateIcono.dibujar(pantalla);
			x1+=40;
		}
		for(int i=0; i<jugador[1].cantidadDeRescates; i++)
		{
			rescateIcono.setx(x2);
			rescateIcono.sety(y);
			rescateIcono.dibujar(pantalla);
			x2-=40;
		}
	}	

	/**********************************************************************************************************************/

	void cargarMensajeDeTriunfo(const t_jugador* jugador)
	{
		SDL_FillRect(pantalla, NULL, SDL_MapRGB(pantalla->format, 0, 0, 0));
		printf("%s ganó la partida\n",jugador->nombre);
		escribirMensaje(jugador->nombre,fuenteTriunfo,colorBlanco,WIDTH/2-(11*strlen(jugador->nombre)),270);
		escribirMensaje("ha ganado la partida...",fuenteTriunfo,colorBlanco,130,350);	
		SDL_Flip(pantalla);
	}

	/**********************************************************************************************************************/

/**********************************************************************************************************************/
/*******************************THREAD DE CREACION DE CONEXION Y ENVIO DE PRIMEROS DATOS*******************************/
/**********************************************************************************************************************/

	void *conectar_Y_ObtenerDatos(void *param)
	{
	 
	 	if(obtenerConexion(&servfd)==1)
		 {
		  printf("\nError al obtener conexion de socket server. Cerrando programa\n");
		  eliminarPID();
		  exit(0);
		 }
	  
		if(conectar(&servfd,ip,puerto)==1)
		 {
		  printf("\nError al conectarse al servidor. Cerrando programa.\n");
		  eliminarPID();
		  exit(0);
		 }

		//RECIBIR CONFIRMACION DE PARTIDA ANTES DE ENVIAR LOS PRIMEROS DATOS
		strcpy(id_jugador.nombre,jugador[0].nombre);

		if(enviarDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
		{
	        printf("\nError al enviar paquete de datos a Servidor de partida. Finalizando el proceso cliente.\n");
	        raise(SIGTERM);
	    }
	    bzero(&id_jugador,sizeof(t_identificacion));
		if(recibirDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
	 	{
	        printf("\nError al recibir paquete de datos del Servidor de partida. Finalizando el proceso cliente por falta de datos.\n");
	        raise(SIGTERM);
	    }

	    //VERIFICAMOS SI EL SERVIDOR CREÓ UNA PARTIDA PARA NOSOTROS
	    if(id_jugador.numero==-1) //Si quedamos afuera del torneo
		{
			SDL_Color White= {255, 255 ,255};
			if(TTF_Init()==-1) 
				{  //inicializa el sistema de fuentes.
					fprintf(stderr, "Error al inicializar SDL_TTF: %s\n",TTF_GetError());
					exit(1);
				}
			TTF_Font *rechazoFuente;
			rechazoFuente=cargarFuente("fonts/Jumpman.ttf",30);
			printf("El cliente se ha quedado afuera \n");
			escribirMensaje("Te quedaste afuera del torneo",rechazoFuente,White,150,570);	
			SDL_Flip(pantalla);
			sleep(3);
			kill(getpid(),SIGTERM);
		}
		if(id_jugador.numero==2)
		{
			//SI EL SERVIDOR CREO UNA PARTIDA, LE ENVIAMOS NUESTROS DATOS
			strcpy(id_jugador.nombre,jugador[0].nombre);
			if(enviarDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
			{
		        printf("\nError al enviar paquete de datos a Servidor de partida. Finalizando el proceso cliente.\n");
		        raise(SIGTERM);
		    }
		    //RECIBIMOS LA INFORMACIÓN NECESARIA PARA ARMAR LA PARTIDA
		    bzero(&movimientos,sizeof(t_movimientos));
			if(recibirDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
		 	{
		        printf("\nError al recibir paquete de datos del Servidor de partida. Finalizando el proceso cliente por falta de datos.\n");
		        raise(SIGTERM);
		    }
			//Se guarda el nombre del otro jugador, el nro de jugador y los recorridos de las llamas
			NRO_JUGADOR=id_jugador.numero;
			RESCATES=id_jugador.cantRescates;
			jugador[NRO_JUGADOR].nroJugador=NRO_JUGADOR;
			if(NRO_JUGADOR==0)
				strcpy(jugador[1].nombre, id_jugador.nombre);
			else
				strcpy(jugador[0].nombre, id_jugador.nombre);
			char aux[100];
			sprintf(aux, "Nro jugador: %d       Enemigo: %s\0",NRO_JUGADOR+1,id_jugador.nombre);
			printf("Soy el nro: %d\tMi enemigo es: %s\nRescates para ganar: %d\n", id_jugador.numero,id_jugador.nombre,RESCATES);
			RESCATES++;
			//
			TTF_Font *confirmacionFuente;
			confirmacionFuente=cargarFuente("fonts/Jumpman.ttf",30);
			escribirMensaje(aux,confirmacionFuente,colorVerde,150,570);
			SDL_Flip(pantalla);
			sleep(3);
		}
		//Los posibles recorridos de las llamas se almacenan en 'id_jugador'
		pthread_exit((void *)0);
	}

	/*****************************************HANDLER DE SEÑALES*******************************************************/

	void handler(int iNumSen,siginfo_t *info,void *ni) //funcion declarada para el
	{                //manejo de las senales recibidas por el proceso demonio.
		int x;	
		//int ERRStream = open("/dev/null", O_WRONLY);
		switch(iNumSen)
		{

			case SIGPIPE: //respuesta a la senal SIGPIPE
						//dup2(2,ERRStream);
						signal(SIGPIPE,SIG_IGN);
						printf("Se perdió la conexión con el servidor.(SIGPIPE)\n");	
						// for(x=0;x<3;x++)
						// 	pthread_cancel(hilo[x]);
						eliminarPID();
						terminarConexion(&servfd);
						//msgctl(msqid,IPC_RMID,0); //elimina la cola de mensajes
						//finalizar=1;
						exit(0);
						break;               //correctamente el proceso

		 	case SIGINT: //respuesta a la senal SIGINT
			            //dup2(STDERR_FILENO,ERRStream);
			            signal(SIGINT,SIG_IGN);
			            printf("Se canceló el programa.(SIGINT)\n");
			            raise(SIGTERM); //se autoenvia la senal SIGTERM para que finalice
			            break;               //correctamente el proceso
		 
		  	case SIGTERM: //caso de finalizacion del programa 
		  				printf("SIGTERM\n");
						// for(x=0;x<3;x++)
						// 	pthread_cancel(hilo[x]);
		  				//dup2(STDERR_FILENO,ERRStream);
						eliminarPID();
						printf("ELIMINÉ PID\n");
						//terminarConexion(&servfd);
						//msgctl(msqid,IPC_RMID,0); //elimina la cola de mensajes
						//finalizar=1;
						//eliminarSemaforo(mtxmonitor, "/mtxmonitor");
						// if(info->si_pid==getpid())
						// 	printf("FIN DEL PROGRAMA\n");
						// else
							printf("Se finalizo el programa.\n");
				        exit(0);
				        break;  //se finaliza el proc. demonio.
	  	}
	}

	/******************************************************************************************************************/

	void esperarNuevaPartida()
	{
		if(finalizar==0)
		{
			bzero(&id_jugador,sizeof(t_identificacion));
			//strcpy(id_jugador.nombre,jugador[0].nombre);
			if(recibirDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
		 	{
		        printf("\nREINICIO DE PARTIDA:\n Error al recibir paquete de datos del Servidor de Torneo. \nFinalizando el proceso cliente por falta de datos.\n");
		        exit(0);
		    }
		    printf("RECIBI: %d\n",id_jugador.numero);
		    //VERIFICAMOS SI EL SERVIDOR CREÓ UNA PARTIDA PARA NOSOTROS
		    if(id_jugador.numero==-1) //Si quedamos afuera del torneo
			{
				printf("Recibí RECHAZO\n");
				if(TTF_Init()==-1) 
				{  //inicializa el sistema de fuentes.
					fprintf(stderr, "Error al inicializar SDL_TTF: %s\n",TTF_GetError());
					exit(1);
				}
				SDL_Color White= {255, 255 ,255};
				TTF_Font *rechazoFuente;
				rechazoFuente=cargarFuente("fonts/Jumpman.ttf",30);
				printf("El cliente se ha quedado afuera \n");
				escribirMensaje("Te quedaste afuera del torneo",rechazoFuente,White,150,570);	
				SDL_Flip(pantalla);
				sleep(3);
				kill(getpid(),SIGTERM);
				//finalizarJuego();
			}
			if(id_jugador.numero==2) //Si pasamos a la proxima ronda
			{
				printf("Recibí CONFIRMACIÓN\n");
				if(NRO_JUGADOR==0)
					strcpy(jugador[1].nombre,jugador[0].nombre);
				else
					strcpy(jugador[0].nombre,jugador[1].nombre);
				//SI EL SERVIDOR CREO UNA PARTIDA, LE ENVIAMOS NUESTROS DATOS
				//bzero(&id_jugador,sizeof(t_identificacion));
				strcpy(id_jugador.nombre,jugador[NRO_JUGADOR].nombre);
				printf("Envío mi nombre: %s\n",id_jugador.nombre);
				if(enviarDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
				{
			        printf("\nError al enviar paquete de datos a Servidor de partida. Finalizando el proceso cliente.\n");
			        raise(SIGTERM);
			    }
			    //RECIBIMOS LA INFORMACIÓN NECESARIA PARA ARMAR LA PARTIDA
			    bzero(&id_jugador,sizeof(t_identificacion));
				if(recibirDatos(&servfd,&id_jugador,sizeof(t_identificacion))==-1)
			 	{
			        printf("\nError al recibir paquete de datos del Servidor de partida. Finalizando el proceso cliente por falta de datos.\n");
			        raise(SIGTERM);
			    }
				//Se guarda el nombre del otro jugador, el nro de jugador y los recorridos de las llamas
				NRO_JUGADOR=id_jugador.numero;
				NRO_LLAMA=0;
				jugador[NRO_JUGADOR].nroJugador=NRO_JUGADOR;
				if(NRO_JUGADOR==0)
					strcpy(jugador[1].nombre, id_jugador.nombre);
				else
					strcpy(jugador[0].nombre, id_jugador.nombre);
				char aux[100];
				sprintf(aux, "Nro jugador: %d       Enemigo: %s\0",NRO_JUGADOR+1,id_jugador.nombre);
				printf("Soy el nro: %d\tMi enemigo es: %s\n", id_jugador.numero,id_jugador.nombre);
				RESCATES++;
				//
				TTF_Font *confirmacionFuente;
				confirmacionFuente=cargarFuente("fonts/Jumpman.ttf",30);
				escribirMensaje(aux,confirmacionFuente,colorVerde,150,570);
				SDL_Flip(pantalla);
				sleep(3);
				//REINICIO DEL JUEGO
				inicializarJuego(0);
				dibujarEscena();
				//RELLENO LA ESTRUCTURA DE COMUNICACIÓN
				//pthread_mutex_lock(&movimientos_mtx);
				bzero(&movimientos,sizeof(t_movimientos));
				movimientos.x=jugador[NRO_JUGADOR].x;
				movimientos.y=jugador[NRO_JUGADOR].y;
				movimientos.numero=NRO_JUGADOR;
				movimientos.frameActual=jugador[NRO_JUGADOR].frameActual;
				movimientos.llama=0;
				movimientos.estado=SIN_CAMBIOS;
				movimientos.barril=0;
				ORIGEN_BARRIL=0;
				printf("Envío: x=%d\ty=%d\n",movimientos.x,movimientos.y);
				//ENVÍO MI INFORMACIÓN AL SERVIDOR
				if(enviarDatos(&servfd,&movimientos,sizeof(t_movimientos))==-1)
	         		printf("\nError al enviar paquete de datos a Servidor Partida.\n");
				printf("Comenzando a jugar\n");
				//flag=1;
			}
			if(id_jugador.numero==3) //Si ganamos el torneo
			{
				printf("Recibí CAMPEÓN\n");
				//finalizar=1;
				SDL_Color White= {0, 0 ,255};
				TTF_Font *rechazoFuente;
				rechazoFuente=cargarFuente("fonts/Jumpman.ttf",50);
				printf("FELICIDADES! Ganaste el torneo!\n");
				escribirMensaje("FELICIDADES! Ganaste el torneo!",rechazoFuente,White,100,500);	
				SDL_Flip(pantalla);
				sleep(3);
				raise(SIGTERM);
			}
		}
	}

/**********************************************************************************************************************/
/***********************************THREAD DE VERIFICACION DE PROCESO MONITOR******************************************/
/**********************************************************************************************************************/

	void *verificaMonitor(void *param)
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

/**********************************************************************************************************************/
/*******************************************THREAD DE CONTROL DE RELOJES***********************************************/
/**********************************************************************************************************************/

	void* controlarRelojes(void*param)
	{
		while(1)
		{
			//Duración del martillo para el jugador 1
			if(jugador[0].martillo==1)
			{
				if(jugador[0].relojMartillo.tiempoCumplido())
				{
					//desactivarMartillo(&jugador[0]);
					jugador[0].martillo=0;
				}
			}
			//Duración del martillo para el jugador 2
			if(jugador[1].martillo==1)
			{
				if(jugador[1].relojMartillo.tiempoCumplido())
				{
					//desactivarMartillo(&jugador[1]);
					jugador[1].martillo=0;
				}
			}
			//Tiempo antes de revivir para el jugador 1
			if(jugador[0].tiempoMuerto.activado())
			{
				jugador[0].frameActual++;
				if(jugador[0].frameActual>jugador[0].frameMax)
					jugador[0].frameActual=jugador[0].frameInicial;
				if(jugador[0].tiempoMuerto.tiempoCumplido())
				{
					jugador[0].tiempoMuerto.finalizar();
					reiniciarJugador(&jugador[0]);
				}
			}
			//Tiempo antes de revivir para el jugador 2
			if(jugador[1].tiempoMuerto.activado())
			{
				jugador[1].frameActual++;
				if(jugador[1].frameActual>jugador[1].frameMax)
					jugador[1].frameActual=jugador[1].frameInicial;
				if(jugador[1].tiempoMuerto.tiempoCumplido())
				{
					jugador[1].tiempoMuerto.finalizar();
					reiniciarJugador(&jugador[1]);
				}
			}
			//Reinicio del juego
			if(tiempoReinicio.activado())
			{
					if(tiempoReinicio.tiempoCumplido())
					{
						tiempoReinicio.finalizar();
						cargarRescateEnPantalla();
						escribirNombreEnPantalla(&jugador[0]);
						escribirNombreEnPantalla(&jugador[1]);
						inicializarJuego(1);
						REINICIANDO=0;
					}
			}
			SDL_Delay(FRECUENCIA_DE_CONTROL_DE_RELOJES);
			//sleep(1);
		}
	}

/**********************************************************************************************************************/
/*****************************************THREAD PARA EL MANEJO DE EVENTOS*********************************************/
/**********************************************************************************************************************/

	void* manejarEventos(void*param)
	{
		int tiempoCiclo;
		SDL_Event evento;
		while(1)
		{
			tiempoCiclo=SDL_GetTicks();
			if(SDL_PollEvent(&evento))		//Verificamos si ocurre un evento en el juego
			{
				Uint8 *keys;
				switch(evento.type)
				{
					case SDL_QUIT:
									finalizar=1;
									break;
					case SDL_KEYDOWN:
									pthread_mutex_lock(&movimientos_mtx);
									keys = SDL_GetKeyState(NULL);
									//TECLA PARA SALIR DEL JUEGO
									if(keys[jugador[NRO_JUGADOR].salir])
									{
										finalizar=1;
										break;
									}
									//JUGADOR 1
									if(jugador[NRO_JUGADOR].activo==0)
										break;
									if(keys[jugador[NRO_JUGADOR].saltar] && jugador[NRO_JUGADOR].y > 0 && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0)
									{
										if(jugador[NRO_JUGADOR].saltando==0)	
										{
											jugador[NRO_JUGADOR].saltando=1;	//BLOQUEAMOS LOS DEMÁS MOVIMIENTOS
											jugador[NRO_JUGADOR].incicioSalto=jugador[NRO_JUGADOR].y;
											jugador[NRO_JUGADOR].alturaSaltada=0;
											jugador[NRO_JUGADOR].avanceHorizontal=0;	//SALTO FIJO
											if(keys[jugador[NRO_JUGADOR].derecha])
											{
												jugador[NRO_JUGADOR].avanceHorizontal=VELOCIDAD_HORIZONTAL_SALTO;	//SALTO HACIA DERECHA
												jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
											}
											else
												if(keys[jugador[NRO_JUGADOR].izquierda])
												{
													jugador[NRO_JUGADOR].avanceHorizontal=(-VELOCIDAD_HORIZONTAL_SALTO);	//SALTO HACIA IZQUIERDA
													jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
												}
											switch(jugador[NRO_JUGADOR].estado)
											{
												case MIRANDO_DER:
																jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
																break;
												case MIRANDO_IZQ:
																jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
																break;
												case DERECHA:
																jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_DER;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
																break;
												case IZQUIERDA:
																jugador[NRO_JUGADOR].estado=SALTO_MIRANDO_IZQ;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
																break;
											}
										}	
									}
									// if(keys[SDLK_r])
									// {
									// 	jugador[NRO_JUGADOR].cantidadDeRescates++;	
									// }

									if(keys[jugador[NRO_JUGADOR].arriba] && jugador[NRO_JUGADOR].y>0)
									{
										subirEscalera(&jugador[NRO_JUGADOR]);
									}
									if(keys[jugador[NRO_JUGADOR].abajo] && jugador[NRO_JUGADOR].y < HEIGHT - marioSprite1.geth() && jugador[NRO_JUGADOR].cayendo==0) 
									{
										bajarEscalera(&jugador[NRO_JUGADOR]);
									}
									if(keys[jugador[NRO_JUGADOR].izquierda] && jugador[NRO_JUGADOR].x > 0 && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0) 	//MOVERSE A LA IZQUIERDA
									{
										moverIzquierda(&jugador[NRO_JUGADOR]);
									}
									if(keys[jugador[NRO_JUGADOR].derecha] && jugador[NRO_JUGADOR].x < WIDTH - marioSprite1.getw() && jugador[NRO_JUGADOR].saltando==0 && jugador[NRO_JUGADOR].estado!=EN_ESCALERA && jugador[NRO_JUGADOR].cayendo==0) 	//MOVERSE A LA DERECHA
									{
										moverDerecha(&jugador[NRO_JUGADOR]);
									}

									pthread_mutex_unlock(&movimientos_mtx);

									//CREAR BARRILES MANUALMENTE
									if(keys[SDLK_b])
									{
										if(crearBarril(id_jugador.recorrerllama1)<0)
											printf("Límite de barriles alcanzado\n");
										else
											printf("Barril creado\n");
									}

									if(keys[SDLK_l])
									{
										if(crearLlama()<0)
											printf("Límite de llamas alcanzado\n");
										else
											printf("Llama creada\n");
									}
									break;

					case SDL_KEYUP:	//En caso de que no se ejecuten acciones, reiniciamos la animación al estado de espera
									if(jugador[NRO_JUGADOR].cayendo==0)
									{
										switch(jugador[NRO_JUGADOR].estado)
										{
											case DERECHA:
															if(jugador[NRO_JUGADOR].avanceHorizontal>0)
															{
																jugador[NRO_JUGADOR].estado=MIRANDO_DER;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
															}
															else
															{
																jugador[NRO_JUGADOR].frameActual=QUIETO_DER;
															}
															break;
											case IZQUIERDA:
															if(jugador[NRO_JUGADOR].avanceHorizontal>0)
															{
																jugador[NRO_JUGADOR].estado=MIRANDO_IZQ;
																actualizarAnimaciones(&jugador[NRO_JUGADOR]);
															}
															else
															{
																jugador[NRO_JUGADOR].frameActual=QUIETO_IZQ;
															}
															break;
											default:
															// jugador[NRO_JUGADOR].estado=MIRANDO_DER;
															// actualizarAnimaciones(&jugador[NRO_JUGADOR]);
															break;
										}
									}
				}
			}
			tiempoCiclo = SDL_GetTicks()-tiempoCiclo;	//Tiempo de finalización del frame
			if(tiempoCiclo < 50)
    			SDL_Delay(Uint32(50-tiempoCiclo));
		}
	}