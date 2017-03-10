#ifndef KEYS_H_
#define KEYS_H_
#include <string.h>
#include <SDL/SDL.h>
int getSDLkey(char*);
char* traducirSDLkeyName(int);
char* toLoweString(char*);
char* toUpperString(char*);
#endif