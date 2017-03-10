#ifndef CSPRITE_H_
#define CSPRITE_H_
#include <SDL/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#define TRUE 1
#define FALSE 0

class CFrame 
{
    public:

        SDL_Surface *img;
        SDL_Surface* surface(){return img;}
        void cargar(const char *path);
        void liberar();
        int getw(){return img->w;}
        int geth(){return img->h;}
};

class CSprite
{
    private:

        int posx, posy;
        int state;
        int nframes;
        int cont;
    public:

        CFrame *sprite;
        CSprite(int nf);
        CSprite();
        void finalizar();
        void agregarFrame(CFrame frame);
        int getFrameActual(){return state;}
        void selframe(int nf);
        int frames() {return cont;}
        void setx(int x) {posx=x;}
        void sety(int y) {posy=y;}
        void addx(int c) {posx+=c;}
        void addy(int c) {posy+=c;}
        int getx() {return posx;}
        int gety() {return posy;}
        int getw() {return sprite[state].img->w;}
        int geth() {return sprite[state].img->h;}
        void dibujar(SDL_Surface *superficie);
        int colision(CSprite sp);
        int superpuesto(CSprite sp);
        int sobreIzq(CSprite sp, int vary);
        int sobreDer(CSprite sp, int vary);
        void cargarCarpeta(const char*,int);  
                                                //Carga todos los frames de una carpeta en un sprite.
                                                //Para esto, los frames deben nombrarse "frame_i", donde "i" es un nro
                                                //entero que incia en 0.
                                                //Los parámetros de la función son: 
                                                //                                  const char* path: ruta a los frames, ej: "img/mono/frame_"
                                                //                                  int cant: la cantidad de frames que se cargaran de la carpeta
};
#endif