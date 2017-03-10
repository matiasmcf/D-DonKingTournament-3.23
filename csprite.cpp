#include "csprite.h"

void CFrame::cargar(const char *path)
{
    img = SDL_LoadBMP(path);
    if(img == NULL)
	{
		printf("No se ha podido cargar la imagen: %s\n", SDL_GetError());
		exit(1);
	}
    SDL_SetColorKey(img, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(img->format, 255, 0, 255));
    //img =SDL_DisplayFormat(img);
}

void CFrame::liberar()
{
    SDL_FreeSurface(img);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

CSprite::CSprite(int nc)
{
    sprite = new CFrame[nc];
    nframes = nc;
    cont = 0;
    state=0;
}

CSprite::CSprite()
{
    int nc = 1;
    sprite = new CFrame[nc];
    nframes = nc;
    cont = 0;
    state=0;
}

void CSprite::finalizar()
{
	int i;
    for(i = 0; i <= nframes-1; i++)
    {
        sprite[i].liberar();
    }
}

void CSprite::agregarFrame(CFrame frame)
{
    if(cont < nframes)
    {
        sprite[cont] = frame;
        cont++;
    }
}

void CSprite::selframe(int nc)
{
    if(nc <= cont)
    {
        state = nc;
    }
}

void CSprite::dibujar(SDL_Surface *superficie)
{
    SDL_Rect dest;
    dest.x = posx;
    dest.y = posy;
    SDL_BlitSurface(sprite[state].img, NULL, superficie, &dest);
}

int CSprite::colision(CSprite sp)
{
    int w1, h1, w2, h2, x1, x2, y1, y2;

    w1 = getw();
    h1 = geth();
    x1 = getx();
    y1 = gety();

    w2 = sp.getw();
    h2 = sp.geth();
    x2 = sp.getx();
    y2 = sp.gety();

    if(((x1 + w1) > x2) && ((y1 + h1) > y2) && ((x2 + w2) > x1) && ((y2 + h2) > y1))
    {
        return TRUE;
    } 
    else 
    {
        return FALSE;
    }
}

int CSprite::superpuesto(CSprite sp)
{
    int w1, h1, w2, h2, x1, x2, y1, y2;
    int dezplazamientoAceptable=1;
    w1 = getw()+2*dezplazamientoAceptable;
    h1 = geth();
    x1 = getx()-dezplazamientoAceptable;
    y1 = gety();

    w2 = sp.getw();
    h2 = sp.geth();
    x2 = sp.getx();
    y2 = sp.gety();

    if((x1<=x2)&&(x1+w1>=x2+w2)&&(y2+h2>=y1)&&(y2-h2<=y1+h1))
    {
        return TRUE;
    } 
    else 
    {
        return FALSE;
    }
}

int CSprite::sobreDer(CSprite sp, int varY)
{
    int w1, h1, w2, h2, x1, x2, y1, y2;
    int efectoSobreViga=3;
    w1 = getw();
    h1 = geth();
    x1 = getx();
    y1 = gety();

    w2 = sp.getw();
    h2 = sp.geth();
    x2 = sp.getx();
    y2 = sp.gety();
    if(x2+w2>=x1 && x2+w2<=x1+w1 && y2+h2>=y1-10 && y2+h2<=y1+h1)
    {
        if(y2+h2==y1+efectoSobreViga)
            return 0;
        if((y2+h2>=y1+varY))
            return 1;//Mario está por debajo de la viga
        else if((y2+h2<=y1+varY))
            return 2;//Mario está flotando sobre la viga
    }
    return -1;
}

int CSprite::sobreIzq(CSprite sp, int varY)
{
    int w1, h1, w2, h2, x1, x2, y1, y2;
    int efectoSobreViga=3;
    w1 = getw();
    h1 = geth();
    x1 = getx();
    y1 = gety();

    w2 = sp.getw();
    h2 = sp.geth();
    x2 = sp.getx();
    y2 = sp.gety();
    if(x2>=x1 && x2<=x1+w1 && y2+h2>=y1-10 && y2+h2<=y1+h1)
    {
        if(y2+h2==y1+efectoSobreViga)
            return 0;
        if((y2+h2>=y1+varY))
            return 1;//Mario está por debajo de la viga
        else if((y2+h2<=y1+varY))
            return 2;//Mario está flotando sobre la viga
    }
    return -1;
}


void CSprite::cargarCarpeta(const char* path, int cant)
{
    char nro[4];
    char extension[6]=".bmp\0";
    char pathAux[40];
    strcpy(pathAux,path);
    CFrame frame;
    for(int x=0;x<cant;x++) //El x indica la cantidad de frames distintos a cargar
    {
        strcpy(pathAux,path);
        sprintf(nro,"%d",x);
        strcat(pathAux,nro);
        strcat(pathAux,extension);
        frame.cargar(pathAux);
        if(cont < nframes)
        {
            sprite[cont] = frame;
            cont++;
         }
    }
    state=0;
}
