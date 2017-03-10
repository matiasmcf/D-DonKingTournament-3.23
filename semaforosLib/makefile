CC= g++
LD= g++
CFLAGS= -c
OFILES= ./semaforos.o
CFILES= ./semaforos.cpp

libsemaforos.a: libsemaforos.a(./semaforos.o)

semaforos.o: $(CFILES)
	$(CC) $(CFLAGS) $(CFILES)

clean:
	rm *.o *.a
