CPP = g++
CPP2 = g++
CC = gcc
LDP = g++
LD= gcc
INCDIR = -I./socketsLib/include -I./semaforosLib/include
LIBS = -lpthread -lsocketsLib -lsemaforos -lSDL -lSDL_ttf -lX11
LIBDIR = -L./socketsLib -L./semaforosLib
OFILES = serverPartida.o serverTorneo.o monitor.o keys.o csprite.o donkey.o ctimer.o
CFILES = serverPartida.cpp serverTorneo.cpp monitor.c keys.c csprite.cpp donkey.c ctimer.cpp

ALL: $(OFILES)
	$(LDP) -o serverPartida serverPartida.o $(LIBDIR) $(LIBS)
	$(LDP) -o serverTorneo serverTorneo.o $(LIBDIR) $(LIBS)
	$(LDP) -o monitor monitor.o $(LIBDIR) $(LIBS)
	$(LDP) -o donkey donkey.o csprite.o keys.o ctimer.o $(LIBDIR) $(LIBS)

$(OFILES): $(CFILES)
	$(CPP) -c serverPartida.cpp $(INCDIR)
	$(CPP) -c serverTorneo.cpp $(INCDIR)
	$(CPP) -c monitor.c $(INCDIR)
	$(CPP) -c keys.c $(INCDIR)
	$(CPP) -c csprite.cpp $(INCDIR)
	$(CPP) -c donkey.c $(INCDIR)
	$(CPP) -c ctimer.cpp $(INCDIR)

clean: 
	rm *.o serverPartida serverTorneo monitor donkey

s: 
	./serverTorneo

c:
	./donkey

magic:
	./prueba_fallida.sh
	clear
