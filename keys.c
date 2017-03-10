#include "keys.h"

char* toLowerString(char* cad)
{
	char* c=cad;
	while(*c!='\0')
	{
		*c=tolower(*c);
		c++;
	}
	return cad;
}

char* toUpperString(char* cad)
{
	char* c=cad;
	while(*c!='\0')
	{
		*c=toupper(*c);
		c++;
	}
	return cad;
}

int getSDLkey(char* _cad)
{
	char *cad;
	cad=toLowerString(_cad);
	//LETRAS
	if(strcmp(cad,"a")==0)
		return SDLK_a;
	if(strcmp(cad,"b")==0)
		return SDLK_b;
	if(strcmp(cad,"c")==0)
		return SDLK_c;
	if(strcmp(cad,"d")==0)
		return SDLK_d;
	if(strcmp(cad,"e")==0)
		return SDLK_e;
	if(strcmp(cad,"f")==0)
		return SDLK_f;
	if(strcmp(cad,"g")==0)
		return SDLK_g;
	if(strcmp(cad,"h")==0)
		return SDLK_h;
	if(strcmp(cad,"i")==0)
		return SDLK_i;
	if(strcmp(cad,"j")==0)
		return SDLK_j;
	if(strcmp(cad,"k")==0)
		return SDLK_k;
	if(strcmp(cad,"l")==0)
		return SDLK_l;
	if(strcmp(cad,"m")==0)
		return SDLK_m;
	if(strcmp(cad,"n")==0)
		return SDLK_n;
	if(strcmp(cad,"o")==0)
		return SDLK_o;
	if(strcmp(cad,"p")==0)
		return SDLK_p;
	if(strcmp(cad,"q")==0)
		return SDLK_q;
	if(strcmp(cad,"r")==0)
		return SDLK_r;
	if(strcmp(cad,"s")==0)
		return SDLK_s;
	if(strcmp(cad,"t")==0)
		return SDLK_t;
	if(strcmp(cad,"u")==0)
		return SDLK_u;
	if(strcmp(cad,"v")==0)
		return SDLK_v;
	if(strcmp(cad,"w")==0)
		return SDLK_w;
	if(strcmp(cad,"x")==0)
		return SDLK_x;
	if(strcmp(cad,"y")==0)
		return SDLK_y;
	if(strcmp(cad,"z")==0)
		return SDLK_z;

	//NÚMEROS
	if(strcmp(cad,"0")==0)
		return SDLK_0;
	if(strcmp(cad,"1")==0)
		return SDLK_1;
	if(strcmp(cad,"2")==0)
		return SDLK_2;
	if(strcmp(cad,"3")==0)
		return SDLK_3;
	if(strcmp(cad,"4")==0)
		return SDLK_4;
	if(strcmp(cad,"5")==0)
		return SDLK_5;
	if(strcmp(cad,"6")==0)
		return SDLK_6;
	if(strcmp(cad,"7")==0)
		return SDLK_7;
	if(strcmp(cad,"8")==0)
		return SDLK_8;
	if(strcmp(cad,"9")==0)
		return SDLK_9;

	//NÚMEROS (KEYPAD)
	if(strcmp(cad,"[0]")==0)
		return SDLK_KP0;
	if(strcmp(cad,"[1]")==0)
		return SDLK_KP1;
	if(strcmp(cad,"[2]")==0)
		return SDLK_KP2;
	if(strcmp(cad,"[3]")==0)
		return SDLK_KP3;
	if(strcmp(cad,"[4]")==0)
		return SDLK_KP4;
	if(strcmp(cad,"[5]")==0)
		return SDLK_KP5;
	if(strcmp(cad,"[6]")==0)
		return SDLK_KP6;
	if(strcmp(cad,"[7]")==0)
		return SDLK_KP7;
	if(strcmp(cad,"[8]")==0)
		return SDLK_KP8;
	if(strcmp(cad,"[9]")==0)
		return SDLK_KP9;

	//FLECHAS
	if(strcmp(cad,"arriba")==0)
		return SDLK_UP;
	if(strcmp(cad,"abajo")==0)
		return SDLK_DOWN;
	if(strcmp(cad,"izquierda")==0)
		return SDLK_LEFT;
	if(strcmp(cad,"derecha")==0)
		return SDLK_RIGHT;
	
	//SIMBOLOS
	if(strcmp(cad,".")==0)
		return SDLK_PERIOD;
	if(strcmp(cad,"-")==0)
		return SDLK_MINUS;
	if(strcmp(cad,",")==0)
		return SDLK_COMMA;

	if(strcmp(cad,"[.]")==0)
		return SDLK_KP_PERIOD;
	if(strcmp(cad,"[-]")==0)
		return SDLK_KP_MINUS;
	if(strcmp(cad,"[/]")==0)
		return SDLK_KP_DIVIDE;
	if(strcmp(cad,"[+]")==0)
		return SDLK_KP_PLUS;
	if(strcmp(cad,"[*]")==0)
		return SDLK_KP_MULTIPLY;
	if(strcmp(cad,"[enter]")==0)
		return SDLK_KP_ENTER;

	//OTROS
	if(strcmp(cad,"space")==0)
		return SDLK_SPACE;
	if(strcmp(cad,"backspace")==0)
		return SDLK_BACKSPACE;
	if(strcmp(cad,"tab")==0)
		return SDLK_TAB;
	if(strcmp(cad,"escape")==0)
		return SDLK_ESCAPE;
	if(strcmp(cad,"rshift")==0)
		return SDLK_RSHIFT;
	if(strcmp(cad,"lshift")==0)
		return SDLK_LSHIFT;
	if(strcmp(cad,"rctrl")==0)
		return SDLK_RCTRL;
	if(strcmp(cad,"lctrl")==0)
		return SDLK_LCTRL;
	if(strcmp(cad,"ralt")==0)
		return SDLK_RALT;
	if(strcmp(cad,"lalt")==0)
		return SDLK_LALT;

	//TECLAS DE FUNCIÓN
	if(strcmp(cad,"f1")==0)
		return SDLK_F1;
	if(strcmp(cad,"f2")==0)
		return SDLK_F2;
	if(strcmp(cad,"f3")==0)
		return SDLK_F3;
	if(strcmp(cad,"f4")==0)
		return SDLK_F4;
	if(strcmp(cad,"f5")==0)
		return SDLK_F5;
	if(strcmp(cad,"f6")==0)
		return SDLK_F6;
	if(strcmp(cad,"f7")==0)
		return SDLK_F7;
	if(strcmp(cad,"f8")==0)
		return SDLK_F8;
	if(strcmp(cad,"f9")==0)
		return SDLK_F9;
	if(strcmp(cad,"f10")==0)
		return SDLK_F10;
	if(strcmp(cad,"f11")==0)
		return SDLK_F11;
	if(strcmp(cad,"f12")==0)
		return SDLK_F12;

	//EN CASO DE NO ENCONTRARSE LA TECLA ESPECIFICADA
	return -1;
}