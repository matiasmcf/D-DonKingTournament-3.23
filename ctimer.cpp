#include "ctimer.h"

CTimer::CTimer()
{
	inicio=SDL_GetTicks();
	duracion=10000;
	activo=0;
}

CTimer::CTimer(int _duracion)
{
	inicio=SDL_GetTicks();
	duracion=_duracion*1000;
	activo=0;
}

int CTimer::tiempoTranscurrido()
{
	return SDL_GetTicks()-inicio;
}

int CTimer::obtenerDuracion()
{
	return duracion/1000;
}

void CTimer::modificarDuracion(int _duracion)
{
	duracion=_duracion*1000;
}

void CTimer::iniciar()
{
	inicio=SDL_GetTicks();
	activo=1;
}

void CTimer::iniciar(int _duracion)
{
	inicio=SDL_GetTicks();
	duracion=_duracion*1000;
	activo=1;
}

int CTimer::tiempoCumplido()
{
	return (SDL_GetTicks()-inicio>=duracion)?1:0;
}

void CTimer::finalizar()
{
	activo=0;
}

int CTimer::activado()
{
	return activo;
}