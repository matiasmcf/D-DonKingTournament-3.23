#ifndef _SOCKETSLIB_H
#define _SOCKETSLIB_H
int obtenerConexion(int *);
int asociarConexion(int *, int);
int escucharConexiones(int *, int);
int aceptarConexion(int *, int *);
int conectar(int *, char *, int);
int recibirDatos(int *, void *, int);
int recibirDatosNoBloqueante(int *, void *, int);
int enviarDatos(int *, void *, int);
int terminarConexion(int *);
#endif
