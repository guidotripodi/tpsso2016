#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "eleccion.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static t_pid siguiente_pid(t_pid pid, int es_ultimo) {
	t_pid res = 0; /* Para silenciar el warning del compilador. */
	printf("para %d - el ultimo es %d\n", pid, es_ultimo);
	if (es_ultimo)
		res = 1;
	else
		res = pid + 1;

	return res;
}

void iniciar_eleccion(t_pid pid, int es_ultimo) {
	/* Completar ac� el algoritmo de inicio de la elecci�n.
	 * Si no est� bien documentado, no aprueba.
	 HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
	 HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
	 */
	int mensaje[2];
	mensaje[0] = pid;
	mensaje[1] = pid;
	MPI_Request request;
	int proximo = siguiente_pid(pid, es_ultimo);
	MPI_Isend(mensaje, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
	//aca lo q hago es crearme la tupla y mandarla iniciando la eleccion
}

void esperar(int segundos) {
	int actual = MPI_Wtime();
	while (MPI_Wtime() <= actual + segundos) {
	}
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout) {
	static t_status status = NO_LIDER;
	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + timeout;
	int proximo = siguiente_pid(pid, es_ultimo);
	int flag, origen;
	int token[2];
	MPI_Status status_mpi;
	MPI_Request request;
	MPI_Request request1;
	pid_t hijito;


	token[0] = token[1] = 1;

	while (ahora < tiempo_maximo) {

		flag = 0;
		MPI_Iprobe(ANY_SOURCE, TAG_OTORGADO, MPI_COMM_WORLD, &flag, &status_mpi);

		if (flag == 1) {
			MPI_Irecv(&token, 2, MPI_INT, ANY_SOURCE, TAG_OTORGADO, COMM_WORLD, &request);
			origen = status_mpi.MPI_SOURCE;
			printf("ack: %d -> %d \n", pid, origen);
			MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request1);
			if (token[0] == pid && token[1] == pid) {
				//soy lider cambio status no se cual 
				status = LIDER;
				continue;
			}
			hijito = fork();
			if (hijito == 0) {
				int enviado = 0;
				int mio[2];
				mio[0] = token[0];
				mio[1] = token[1];
				if (mio[0] == pid) {
					if (mio[1] > pid) {
						//el lider esta mas adelante
						//mando msj con cl cl

						mio[0] = mio[1];
						mio[1] = mio[1];
						printf("el lider es otro: %d -> %d  token: {%d,%d}\n", pid, proximo, mio[0], mio[1]);

					}
				} else {
					//todavia no termino
					if (mio[1] < pid) {
						//sigue con un nuevo cl
						mio[1] = pid;
					}
					printf("token ajeno: %d -> %d token: {%d,%d}\n", pid, proximo, mio[0], mio[1]);

				}
				while (enviado == 0) {
					printf("envio un token: %d -> %d token: {%d,%d}\n", pid, proximo, mio[0], mio[1]);
					MPI_Isend(&mio, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
					flag = 0;

					while (flag == 0 && MPI_Wtime() <= ahora + 2) {
						MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status_mpi);
					}
					if (flag == 1) {
						MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request);
						enviado = 1;
					} else {
						proximo++;
					}
				}
				exit(0);
			}



		}


		/* Actualizo valor de la hora. */

		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
}


