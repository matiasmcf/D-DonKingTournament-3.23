#ifndef CTIMER_H_
#define CTIMER_H_
#include <SDL/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

class CTimer
{
	private:
			int inicio;
			int duracion;
			int activo;
	public:
			CTimer();	//Constructor por defecto (Duración_ 10)
			CTimer(int);	//Constructor parametrizado
			int tiempoTranscurrido();
			int obtenerDuracion();
			void modificarDuracion(int);
			void iniciar();
			void iniciar(int);
			int tiempoCumplido();
			void finalizar();
			int activado();
};

#endif