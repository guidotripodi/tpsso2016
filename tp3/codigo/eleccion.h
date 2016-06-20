#ifndef	_ELECCION_H
#define	_ELECCION_H

#define	TAG_OTORGADO 20
#define	TAG_ACK	30
#define	TAG_CONTROL	10

#define COMM_WORLD      MPI_COMM_WORLD
#define ANY_SOURCE      MPI_ANY_SOURCE
#define ANY_TAG         MPI_ANY_TAG

/* Enumerado para ver si soy o no líder. */
enum status { NO_LIDER, LIDER };
typedef enum status t_status;

/* PID de elección de líder. */
typedef unsigned short int t_pid;
#define	MPI_PID	MPI_UNSIGNED_SHORT

/* Función que maneja la elección de líder por no más
 * de timeout segundos.
 * pid es el PID del proceso local.
 * es_ultimo indica si éste es el último proceso del anillo o no.
 */
void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout);
void esperar(double ahora, unsigned int segundos);

/* Función que se encarga de iniciar la elección de líder.
 * Los parámetros tienen la misma semántica que eleccion_lider().
 */
void iniciar_eleccion(t_pid pid, int es_ultimo);

#endif	/* ELECCION_H */
 