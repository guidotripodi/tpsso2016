#ifndef Backend_multi_h
#define Backend_multi_h

#include <pthread.h>
#include "Encabezado.h"
#include "Casillero.h"
#include "Enviar_recibir.h"
#include "RWLock.h"


struct thread_data{
   int  socket_cliente_struct;
   RWLock rw_lock;
};

using namespace std;
bool cargar_int(const char* numero, unsigned int& n);

void *atendedor_de_jugador(void *socket_param) ;


// mensajes recibidos por el server
int enviar_error_equipo_de_mas(int socket_fd);
int recibir_nombre_equipo(int socket_fd, char* nombre);
int recibir_comando(int socket_fd, char* mensaje);
int parsear_barco(char* mensaje, Casillero& ficha);
int parsear_bomba(char* mensaje, Casillero& ficha);


// mensajes enviados por el server
int enviar_dimensiones(int socket_fd);
int enviar_tablero(int socket_fd , bool soy_equipo_1);
int enviar_ok(int socket_fd);
int enviar_error(int socket_fd);
int enviar_golpe(int socket_fd);
int enviar_estaba_golpeado(int socket_fd);


// otras funciones
void cerrar_servidor(int signal);
void terminar_servidor_de_jugador(int socket_fd, list<Casillero>& barco_actual, vector<vector<char> >& tablero_cliente);

void quitar_partes_barco(list<Casillero>& barco_actual, vector<vector<char> >& tablero_cliente);
bool es_ficha_valida(const Casillero& ficha, const list<Casillero>& barco_actual, const vector<vector<char> >& tablero);
Casillero casillero_mas_distante_de(const Casillero& ficha, const list<Casillero>& barco_actual);
bool puso_barco_en(unsigned int fila, unsigned int columna, const list<Casillero>& barco_actual);


bool registrar_equipo (char* nombre);

#endif
