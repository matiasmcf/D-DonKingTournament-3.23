CC= g++
LD= g++
CFLAGS= -c
OFILES= ./socketsLib.o
CFILES= ./socketsLib.cpp

libsocketsLib.a: libsocketsLib.a(./socketsLib.o)

socketsLib.o: $(CFILES)
	$(CC) $(CFLAGS) socketsLib.cpp

clean:
	rm *.o *.a
