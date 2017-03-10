#ifndef _SEMAFOROS_H
#define _SEMAFOROS_H
sem_t *obtenerMutex(const char *nombre); //Devuelve el Id del Semaforo Mutex creado.
sem_t *obtenerMutexEn0(const char *nombre); //Devuelve el Id del Semaforo Mutex creado inicializado en 0.
sem_t *obtenerSemaforo(const char *nombre, unsigned int valor); //Devuelve el Id del semaforo creado.
void pedirSemaforo(sem_t *sem); //P(), Aumenta en 1 el valor del semaforo pasado por parametro.
void devolverSemaforo(sem_t *sem); //V(), Decrementa en 1 el valor del semaforo pasado por parametro.
void eliminarSemaforo(sem_t *sem, const char *nombre); //Elimina el semaforo pasado por parametro.
#endif
