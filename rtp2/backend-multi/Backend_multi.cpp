#include "Backend_multi.h"


using namespace std;


// variables globales de la conexión
int socket_servidor = -1;

// variables globales del juego
vector<vector<char> > tablero_equipo1; // tiene las fichas del equipo1
vector<vector<char> > tablero_equipo2; // tiene las fichas del equipo2

bool peleando = false;
int listos = 0;
int jugadores = 0;
unsigned int ancho = -1;
unsigned int alto = -1;
vector<vector<RWLock> > rwlocks_tablero_equipo1;
vector<vector<RWLock> > rwlocks_tablero_equipo2;
pthread_mutex_t equipo1;

pthread_mutex_t equipos_mutex;
//pthread_mutex_t peleando_mutex;
pthread_mutex_t socket_mutex;


char equipos[2][21];
bool equiposeleccionado[2];

RWLock rwlock_tablero_equipo1;
RWLock rwlock_tablero_equipo2;
RWLock rwlock_peleando;

bool registrar_equipo(char* nombre) {
	int res;
	bool registrado = false;
	pthread_mutex_lock(&equipos_mutex);
	if (equiposeleccionado[0]) {
		res = strcmp(nombre, equipos[0]);
		//cerr << "equipo0: " << equipos[0] << "res:" << res << endl;
		registrado = (res == 0);
	} else {
		strcpy(equipos[0], nombre);
		equiposeleccionado[0] = true;
		//cerr << "equipo0: " << equipos[0] <<  endl;
		registrado = true;
	}
	if (!registrado) {
		if (equiposeleccionado[1]) {
			res = strcmp(nombre, equipos[1]);
			registrado = (res == 0);
		} else {
			strcpy(equipos[1], nombre);
			equiposeleccionado[1] = true;
			registrado = true;
		}
	}
	pthread_mutex_unlock(&equipos_mutex);
	return registrado;
}

bool cargar_int(const char* numero, unsigned int& n) {
	char *eptr;
	n = static_cast<unsigned int> (strtol(numero, &eptr, 10));
	if (*eptr != '\0') {
		cerr << "error: " << numero << " no es un número: " << endl;
		return false;
	}
	return true;
}

int main(int argc, const char* argv[]) {

	// manejo la señal SIGINT para poder cerrar el socket cuando cierra el programa
	signal(SIGINT, cerrar_servidor);

	// parsear argumentos
	if (argc < 3) {
		cerr << "Faltan argumentos, la forma de uso es:" << endl <<
				argv[0] << " N M" << endl << "N = ancho del tablero , M = alto del tablero" << endl;
		return 3;
	} else {
		if (!cargar_int(argv[1], ancho)) {
			cerr << argv[1] << " debe ser un número" << endl;
			return 5;
		}
		if (!cargar_int(argv[2], alto)) {
			cerr << argv[2] << " debe ser un número" << endl;
			return 5;
		}
	}

	// inicializar ambos tableros, se accede como tablero[fila][columna]
	tablero_equipo1 = vector<vector<char> >(alto);
	rwlocks_tablero_equipo1 = vector<vector<RWLock> >(alto);
	for (unsigned int i = 0; i < alto; ++i) {
		tablero_equipo1[i] = vector<char>(ancho, VACIO);
		rwlocks_tablero_equipo1[i] = vector<RWLock>(ancho);
	}

	tablero_equipo2 = vector<vector<char> >(alto);
	rwlocks_tablero_equipo2 = vector<vector<RWLock> >(alto);
	for (unsigned int i = 0; i < alto; ++i) {
		tablero_equipo2[i] = vector<char>(ancho, VACIO);
		rwlocks_tablero_equipo2[i] = vector<RWLock>(ancho);
	}
	pthread_mutex_lock(&equipos_mutex);
	equiposeleccionado[0] = false;
	equiposeleccionado[1] = false;
	pthread_mutex_unlock(&equipos_mutex);

	int socketfd_cliente, socket_size;
	struct sockaddr_in local, remoto;

	// crear un socket de tipo INET con TCP (SOCK_STREAM)
	if ((socket_servidor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		cerr << "Error creando socket" << endl;
	}

	// permito reusar el socket para que no tire el error "Address Already in Use"
	int flag = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag));

	// crear nombre, usamos INADDR_ANY para indicar que cualquiera puede conectarse aquí
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(PORT);
	if (bind(socket_servidor, (struct sockaddr *) &local, sizeof (local)) == -1) {
		cerr << "Error haciendo bind!" << endl;
		return 1;
	}

	// escuchar en el socket
	if (listen(socket_servidor, 1) == -1) {
		cerr << "Error escuchando socket!" << endl;
		return 1;
	}
	pthread_t threads[NUM_THREADS];
	// aceptar conexiones entrantes.
	socket_size = sizeof (remoto);
	int i = 0;
	while (true) {
		pthread_mutex_lock(&socket_mutex);
		if ((socketfd_cliente = accept(socket_servidor, (struct sockaddr*) &remoto, (socklen_t*) & socket_size)) == -1) {
			pthread_mutex_unlock(&socket_mutex);
			cerr << "Error al aceptar conexion" << endl;
		} else {
			
			pthread_create(&threads[i], NULL, &atendedor_de_jugador, &socketfd_cliente);
			i++;
			//atendedor_de_jugador(socketfd_cliente);
		}
	}

	close(socket_servidor);
	pthread_exit(NULL);
	return 0;
}

void *atendedor_de_jugador(void *socket_param) {
	// variables locales del jugador
	char nombre_equipo[21];
	int socket_fd = *((int *) socket_param);
	pthread_mutex_unlock(&socket_mutex);
	bool listo = false;
	bool soy_equipo_1 = true;
	// lista de casilleros que ocupa el barco actual (aún no confirmado)
	list<Casillero> barco_actual;

	rwlock_peleando.rlock();
	if (peleando) {
		
		cout << "Llegaste tarde ya empezo la batalla" << endl;
		if (enviar_error(socket_fd) != 0) {
			// se produjo un error al enviar. Cerramos todo.
			// no quiero vivir mas
		}
		rwlock_peleando.runlock();
		return NULL;
	} 
	rwlock_peleando.runlock();

	if (recibir_nombre_equipo(socket_fd, nombre_equipo) != 0) {
		// el cliente cortó la comunicación, o hubo un error. Cerramos todo.
		terminar_servidor_de_jugador(socket_fd, barco_actual, tablero_equipo1);
	}



	if (!registrar_equipo(nombre_equipo)) {
		//el nombre del equipo es invalido
		cout << "Ya hay dos equipos en la batalla" << endl;
		if (enviar_error(socket_fd) != 0) {
			// se produjo un error al enviar. Cerramos todo.
			// no quiero vivir mas
		}
		return NULL;
	}




	soy_equipo_1 = strcmp(equipos[0], nombre_equipo) == 0;
	pthread_mutex_lock(&equipo1);
	jugadores++;
	pthread_mutex_unlock(&equipo1);
	if (enviar_dimensiones(socket_fd) != 0) {
		// se produjo un error al enviar. Cerramos todo.
		terminar_servidor_de_jugador(socket_fd, barco_actual, tablero_equipo1);
	}



	cout << "Esperando que juegue " << nombre_equipo << endl;

	//Hay un solo equipo, soy siempre el equipo1


	vector<vector<char> > *tablero_jugador;
	vector<vector<char> > *tablero_rival;
	vector<vector<RWLock> > *rwlocks_tablero;
	vector<vector<RWLock> > *rwlocks_tablero_rival;

	//Veo que tablero usar dependiendo el equipo que soy
	if (!soy_equipo_1) {
		tablero_jugador = &tablero_equipo2;
		rwlocks_tablero = &rwlocks_tablero_equipo2;
		tablero_rival = &tablero_equipo1;
		rwlocks_tablero_rival = &rwlocks_tablero_equipo1;
	} else {
		tablero_jugador = &tablero_equipo1;
		rwlocks_tablero = &rwlocks_tablero_equipo1;
		tablero_rival = &tablero_equipo2;
		rwlocks_tablero_rival = &rwlocks_tablero_equipo2;
	}

	while (true) {

		// espera un barco o confirmación de juego
		char mensaje[MENSAJE_MAXIMO + 1];
		int comando = recibir_comando(socket_fd, mensaje);

		if (comando == MSG_PARTE_BARCO) {
			Casillero ficha;

			//Si estoy peleando, no acepto barcos ya
			if (listo) {
				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}

				continue;
			}

			if (parsear_barco(mensaje, ficha) != 0) {
				// no es un mensaje PARTE_BARCO bien formado, hacer de cuenta que nunca llegó
				continue;
			}

			// ficha contiene el nuevo barco a colocar
			// verificar si es una posición válida del tablero
			if (es_ficha_valida(ficha, barco_actual, *tablero_jugador)) {
				barco_actual.push_back(ficha);

				(*rwlocks_tablero)[ficha.fila][ficha.columna].wlock();
				(*tablero_jugador)[ficha.fila][ficha.columna] = ficha.contenido;
				(*rwlocks_tablero)[ficha.fila][ficha.columna].wunlock();

				// OK
				if (enviar_ok(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}
			} else {
				// ERROR
				quitar_partes_barco(barco_actual, *tablero_jugador);

				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}
			}
		} else if (comando == MSG_LISTO) {
			// El único cliente terminó de ubicar sus barcos

			//Si ya había terminado, enviar error
			if (listo) {

				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}

			} else {
				// Estamos listos para la pelea
				listo = true;
				//protejo variable global listos jugadores y peleando
				rwlock_peleando.wlock();
				listos++;
				if (listos == jugadores){
					peleando = true;
				}
				rwlock_peleando.wunlock();

				if (enviar_ok(socket_fd) != 0)
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
			}

		} else if (comando == MSG_BARCO_TERMINADO) {


			//Si estoy peleando, no acepto barcos ya
			if (listo) {
				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}

				continue;
			}



			//bloqueo mi tablero entero
			
			// las partes acumuladas conforman un barco completo, escribirlas en el tablero del jugador y borrar las partes temporales
			for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
				(*rwlocks_tablero)[casillero->fila][casillero->columna].wlock();
				(*tablero_jugador)[casillero->fila][casillero->columna] = casillero->contenido;
				(*rwlocks_tablero)[casillero->fila][casillero->columna].wunlock();
			}
			barco_actual.clear();



			if (enviar_ok(socket_fd) != 0) {
				// se produjo un error al enviar. Cerramos todo.
				terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
			}
		} else if (comando == MSG_BOMBA) {
			//TODO

			Casillero ficha;
			if (parsear_bomba(mensaje, ficha) != 0) {
				// no es un mensaje BOMBA bien formado, hacer de cuenta que nunca llegó
				continue;
			}

			// ficha contiene la bomba a tirar
			// verificar si se está peleando y si es una posición válida del tablero
			if (peleando && ficha.fila <= alto - 1 && ficha.columna <= ancho - 1) {

				//Si había un BARCO, pongo una BOMBA
				(*rwlocks_tablero_rival)[ficha.fila][ficha.columna].wlock();
				char contenido = (*tablero_rival)[ficha.fila][ficha.columna];

				if (contenido == BARCO) {

					(*tablero_rival)[ficha.fila][ficha.columna] = BOMBA;
					(*rwlocks_tablero_rival)[ficha.fila][ficha.columna].wunlock();
					if (enviar_golpe(socket_fd) != 0) {
						// se produjo un error al enviar. Cerramos todo.
						terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
					}

				} else if (contenido == BOMBA) {
					(*rwlocks_tablero_rival)[ficha.fila][ficha.columna].wunlock();
					// OK
					if (enviar_estaba_golpeado(socket_fd) != 0) {
						// se produjo un error al enviar. Cerramos todo.
						terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
					}
				} else {
					(*rwlocks_tablero_rival)[ficha.fila][ficha.columna].wunlock();
					// OK
					if (enviar_ok(socket_fd) != 0) {
						// se produjo un error al enviar. Cerramos todo.
						terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
					}
				}
			} else {
				// ERROR
				if (enviar_error(socket_fd) != 0) {
					// se produjo un error al enviar. Cerramos todo.
					terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
				}
			}

		} else if (comando == MSG_UPDATE) {
			if (enviar_tablero(socket_fd, soy_equipo_1) != 0) {
				// se produjo un error al enviar. Cerramos todo.
				terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
			}
		} else if (comando == MSG_INVALID) {
			// no es un mensaje válido, hacer de cuenta que nunca llegó
			continue;
		} else {
			// se produjo un error al recibir. Cerramos todo.
			terminar_servidor_de_jugador(socket_fd, barco_actual, *tablero_jugador);
		}
	}
	pthread_exit(NULL);
	return NULL;
}


// mensajes recibidos por el server

int recibir_nombre_equipo(int socket_fd, char* nombre) {
	char buf[MENSAJE_MAXIMO + 1];

	if (recibir(socket_fd, buf) != 0) {
		return -1;
	}

	int res = sscanf(buf, "EQUIPO %20s", nombre);

	if (res == EOF || res != 1) {
		cerr << "ERROR: no se pudo leer el nombre del equipo" << endl;
		return -1;
	}

	return 0;
}

// informa el tipo de comando recibido (o si es inválido)
// deja el mensaje en mensaje por si necesita seguir parseando

int recibir_comando(int socket_fd, char* mensaje) {
	if (recibir(socket_fd, mensaje) != 0) {
		return -1;
	}

	char comando[MENSAJE_MAXIMO];
	sscanf(mensaje, "%s", comando);

	if (strcmp(comando, "PARTE_BARCO") == 0) {
		// el mensaje es PARTE_BARCO
		return MSG_PARTE_BARCO;
	} else if (strcmp(comando, "BARCO_TERMINADO") == 0) {
		// el mensaje es BARCO_TERMINADO
		return MSG_BARCO_TERMINADO;
	} else if (strcmp(comando, "LISTO") == 0) {
		// el mensaje es LISTO
		return MSG_LISTO;
	} else if (strcmp(comando, "BOMBA") == 0) {
		// el mensaje es BOMBA
		return MSG_BOMBA;
	} else if (strcmp(comando, "UPDATE") == 0) {
		// el mensaje es UPDATE
		return MSG_UPDATE;
	} else {
		cerr << "ERROR: mensaje no válido" << endl;
		return MSG_INVALID;
	}
}

int parsear_bomba(char* mensaje, Casillero& ficha) {

	int cant = sscanf(mensaje, "BOMBA %d %d", &ficha.fila, &ficha.columna);
	ficha.contenido = BOMBA;

	if (cant == 2) {
		//El mensaje BARCO es válido
		return 0;
	} else {
		cerr << "ERROR: " << mensaje << " no está bien formado. Debe ser BOMBA <fila> <columna>" << endl;
		return -1;
	}
}

int parsear_barco(char* mensaje, Casillero& ficha) {

	int cant = sscanf(mensaje, "PARTE_BARCO %d %d", &ficha.fila, &ficha.columna);
	ficha.contenido = BARCO;

	if (cant == 2) {
		//El mensaje PARTE_BARCO es válido
		return 0;
	} else {
		cerr << "ERROR: " << mensaje << " no está bien formado. Debe ser PARTE_BARCO <fila> <columna>" << endl;
		return -1;
	}
}



// mensajes enviados por el server

int enviar_dimensiones(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "TABLERO %d %d", ancho, alto);
	return enviar(socket_fd, buf);
}

int enviar_tablero(int socket_fd, bool soy_equipo_1) {
	char buf[MENSAJE_MAXIMO + 1];
	int pos;
	vector<vector<char> > *tablero;
	RWLock *rwlock_tablero;
	

	//Si no estoy peleando, muestro los barcos de mi equipo
	rwlock_peleando.rlock();
	if (!peleando) {
		sprintf(buf, "BARCOS ");
		pos = 7;
		
	} else {
		//Sino muestro los resultados de la batalla
		sprintf(buf, "BATALLA ");

		pos = 8;
		
	}
	
	if (peleando ^ soy_equipo_1) {
		tablero = &tablero_equipo1;
		rwlock_tablero = &rwlock_tablero_equipo1;
	} else {
		tablero = &tablero_equipo2;
		rwlock_tablero = &rwlock_tablero_equipo2;
	}
	rwlock_peleando.runlock();


	(*rwlock_tablero).rlock();
	for (unsigned int fila = 0; fila < alto; ++fila) {
		for (unsigned int col = 0; col < ancho; ++col) {
			char contenido = (*tablero)[fila][col];
			switch (contenido) {
				case VACIO:
					buf[pos] = '-';
					break; //optional
				case BARCO:
					//si estoy peleando, oculto los barcos. Sino, los muestro
					buf[pos] = peleando ? '-' : 'B';
					break; //optional
				case BOMBA:
					buf[pos] = '*';
			}

			pos++;
		}
	}
	buf[pos] = 0; //end of buffer
	cout << endl;
	(*rwlock_tablero).runlock();
	
	
	return enviar(socket_fd, buf);
}

int enviar_ok(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "OK");
	return enviar(socket_fd, buf);
}

int enviar_golpe(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "GOLPE");
	return enviar(socket_fd, buf);
}

int enviar_estaba_golpeado(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "ESTABA_GOLPEADO");
	return enviar(socket_fd, buf);
}

int enviar_error(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "ERROR");
	return enviar(socket_fd, buf);
}

int enviar_error_equipo_de_mas(int socket_fd) {
	char buf[MENSAJE_MAXIMO + 1];
	sprintf(buf, "ERROR EQUIPO NO VALIDO");
	return enviar(socket_fd, buf);
}


// otras funciones

void cerrar_servidor(int signal) {
	cout << "¡Adiós mundo cruel!" << endl;
	if (socket_servidor != -1)
		close(socket_servidor);
	exit(EXIT_SUCCESS);
}

void terminar_servidor_de_jugador(int socket_fd, list<Casillero>& barco_actual, vector<vector<char> >& tablero_cliente) {

	cout << "Se interrumpió la comunicación con un cliente" << endl;

	close(socket_fd);

	quitar_partes_barco(barco_actual, tablero_cliente);

	exit(-1);
}

void quitar_partes_barco(list<Casillero>& barco_actual, vector<vector<char> >& tablero_cliente) {
	for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
		tablero_cliente[casillero->fila][casillero->columna] = VACIO;
	}
	barco_actual.clear();
}

bool es_ficha_valida(const Casillero& ficha, const list<Casillero>& barco_actual, const vector<vector<char> >& tablero) {

	// si está fuera del tablero, no es válida
	if (ficha.fila > alto - 1 || ficha.columna > ancho - 1) {
		return false;
	}

	// si el casillero está ocupado, tampoco es válida
	if (tablero[ficha.fila][ficha.columna] != VACIO) {
		return false;
	}

	if (barco_actual.size() > 0) {
		// no es la primera parte del barco, ya hay fichas colocadas para este barco
		Casillero mas_distante = casillero_mas_distante_de(ficha, barco_actual);
		int distancia_vertical = ficha.fila - mas_distante.fila;
		int distancia_horizontal = ficha.columna - mas_distante.columna;

		if (distancia_vertical == 0) {
			// el barco es horizontal
			for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
				if (ficha.fila - casillero->fila != 0) {
					// no están alineadas horizontalmente
					return false;
				}
			}

			int paso = distancia_horizontal / abs(distancia_horizontal);
			for (unsigned int columna = mas_distante.columna; columna != ficha.columna; columna += paso) {
				// el casillero DEBE estar ocupado en el tablero
				if (!(puso_barco_en(ficha.fila, columna, barco_actual)) && tablero[ficha.fila][columna] == VACIO) {
					return false;
				}
			}

		} else if (distancia_horizontal == 0) {
			// el barco es vertical
			for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
				if (ficha.columna - casillero->columna != 0) {
					// no están alineadas verticalmente
					return false;
				}
			}

			int paso = distancia_vertical / abs(distancia_vertical);
			for (unsigned int fila = mas_distante.fila; fila != ficha.fila; fila += paso) {
				// el casillero DEBE estar ocupado en el tablero
				if (!(puso_barco_en(fila, ficha.columna, barco_actual)) && tablero[fila][ficha.columna] == VACIO) {
					return false;
				}
			}
		} else {
			// no están alineadas ni horizontal ni verticalmente
			return false;
		}
	}

	return true;
}

Casillero casillero_mas_distante_de(const Casillero& ficha, const list<Casillero>& barco_actual) {
	const Casillero* mas_distante;
	int max_distancia = -1;
	for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
		int distancia = max<unsigned int>(abs((int) (casillero->fila - ficha.fila)), abs((int) (casillero->columna - ficha.columna)));
		if (distancia > max_distancia) {
			max_distancia = distancia;
			mas_distante = &*casillero;
		}
	}

	return *mas_distante;
}

bool puso_barco_en(unsigned int fila, unsigned int columna, const list<Casillero>& barco_actual) {
	for (list<Casillero>::const_iterator casillero = barco_actual.begin(); casillero != barco_actual.end(); casillero++) {
		if (casillero->fila == fila && casillero->columna == columna)
			return true;
	}
	// si no encontró
	return false;
}

