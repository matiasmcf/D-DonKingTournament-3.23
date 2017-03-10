// Nombre de Archivo: semaforos.cpp
// Trabajo Practico 4
// 
// Integrantes:
//	ARIEL
//	AGUSTIN
//	FACUNDO
//  MATIAS
//	Rodriguez Viano, Sebastian	Dni: 35.361.326
// Entrega Numero 1
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <sys/types.h>  
#include <sys/wait.h>
#include <string.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>


/*union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};*/


sem_t *obtenerMutex(const char *nombre)
{
 sem_t *sem;

sem = sem_open(nombre, O_CREAT, 0777, 1);

	if(sem == SEM_FAILED)
	{
		printf("Error al crear el semaforo: %s\n", strerror(errno));
		exit(1);
	}

	return sem;
}

sem_t *obtenerMutexEn0(const char *nombre)
{
 sem_t *sem;

sem = sem_open(nombre, O_CREAT, 0777, 0);

	if(sem == SEM_FAILED)
	{
		fprintf(stderr, "Error al crear el semaforo: %s\n", strerror(errno));
		exit(1);
	}

	return sem;
}

sem_t *obtenerSemaforo(const char *nombre, unsigned int valor)
{
 sem_t *sem;

sem = sem_open(nombre, O_CREAT, 0777, valor);

	if(sem == SEM_FAILED)
	{
		fprintf(stderr, "Error al crear el semaforo: %s\n", strerror(errno));
		exit(1);
	}

	return sem;
}

void pedirSemaforo(sem_t *sem)
{
sem_wait(sem); 
//FALTARIA VALIDAR SI SE ELIMINA EL SEMAFORO. ARROJARIA EINVAL
}

void devolverSemaforo(sem_t *sem)
{
sem_post(sem); //IDEM. FALTA VALIDACION
}

void eliminarSemaforo(sem_t *sem, const char *nombre)
{
	sem_close(sem);
	sem_unlink(nombre);
}


