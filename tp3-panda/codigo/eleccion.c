#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"
#include <list>


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
	int flag, origen, tag;
	int token[2];
	MPI_Status status_mpi;
	MPI_Request request;
	MPI_Request request1;

	token[0] = token[1] = 1;

	while (ahora < tiempo_maximo) {

		flag = 0;
		MPI_Iprobe(ANY_SOURCE, ANY_TAG, MPI_COMM_WORLD, &flag, &status_mpi);

		if (flag == 1) {
			MPI_Irecv(&token, 2, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &request);
			origen = status_mpi.MPI_SOURCE;
			tag = status_mpi.MPI_TAG;

			if (tag == TAG_OTORGADO) {
				printf("ack: %d -> %d \n", pid, origen);
				MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request1);
				if (token[0] == pid) {
					if (token[1] == pid) {
						//soy lider cambio status no se cual 
						status = LIDER;
					}
					if (token[1] > pid) {
						//el lider esta mas adelante
						//mando msj con cl cl
		
						token[0] = token[1];
						token[1] = token[1];
						printf("el lider es otro: %d -> %d  token: {%d,%d}\n", pid, proximo, token[0], token[1]);
						MPI_Isend(token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
					}
				}else {
					//todavia no termino
					if (token[1] < pid) {
						//sigue con un nuevo cl
						token[1] = pid;
					}
					printf("token ajeno: %d -> %d token: {%d,%d}\n", pid, proximo, token[0], token[1]);
					MPI_Isend(token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
				}

				continue;
			}
			if (tag == TAG_ACK) {
				//removerOrigen(origen);
			}
		}else{
			if (ahora != MPI_Wtime()){
				//revisarSiHayQueReenviar();
			}
		}


		/* Actualizo valor de la hora. */

		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
}


