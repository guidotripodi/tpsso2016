CC = g++
CFLAGS = -g -Wall -Werror
LDFLAGS = -pthread

BINARIES = backend rwlocktest

all: $(BINARIES)

backend: Backend_multi.cpp Backend_multi.h Enviar_recibir.o Enviar_recibir.h Encabezado.h Casillero.h 
	$(CC) $(LDFLAGS) $(CFLAGS) RWLock.cpp Backend_multi.cpp Enviar_recibir.o -o backend

Enviar_recibir.o: Enviar_recibir.cpp Enviar_recibir.h Encabezado.h
	$(CC) $(CFLAGS) -c Enviar_recibir.cpp

rwlocktest: RWLockTest.cpp RWLock.cpp RWLock.h
	$(CC) $(LDFLAGS) $(CFLAGS) RWLock.cpp RWLockTest.cpp -o rwlocktest

clean:
	rm -f *.o
	rm -f $(BINARIES)
