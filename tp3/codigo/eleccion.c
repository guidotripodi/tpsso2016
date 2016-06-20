#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo) {
	t_pid res = 0; /* Para silenciar el warning del compilador. */
	//printf("para %d - el ultimo es %d\n", pid, es_ultimo);
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

void esperar(double ahora, unsigned int segundos) {
	double actual = ahora;
	actual = MPI_Wtime();
	while (actual < ahora + segundos) {
		actual = MPI_Wtime();
	}
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout) {
	static t_status status = NO_LIDER;
	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + timeout;
	//double actual = 0;
	t_pid proximo = 0;
	proximo = siguiente_pid(pid, es_ultimo);
	int flag, origen, cont;
	int buffer[2];
	MPI_Status status_mpi;
	MPI_Request request;
	MPI_Request request1;
	request = 0;
	request1 = 0;
	buffer[0] = buffer[1] = 1;
	origen = 1;
	cont = 0;

	while (ahora < tiempo_maximo) {
		flag = 0;

		while (flag == 0 && MPI_Wtime() < tiempo_maximo) {
			MPI_Iprobe(ANY_SOURCE, TAG_OTORGADO, MPI_COMM_WORLD, &flag, &status_mpi);
		}

		MPI_Irecv(&buffer, 2, MPI_INT, ANY_SOURCE, TAG_OTORGADO, COMM_WORLD, &request);
		origen = status_mpi.MPI_SOURCE;

		//printf("ack: %d -> %d \n", pid, origen);
		MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request1);



		if (buffer[0] == pid) {
			//hay lider
			if (buffer[1] == pid) {
				//soy lider cambio status y termino mi eleccion 
				status = LIDER;
			}
			if (buffer[1] > pid) {
				//el lider esta mas adelante
				//mando msj con cl cl

				buffer[0] = buffer[1];
				buffer[1] = buffer[1];
				flag = 0;


				while (flag == 0) { //aca tengo q esperar t segundos q no se como poner eso 
					printf("mensaje a enviar: %d -> %d de: %d a %d \n", buffer[0], buffer[1], pid , proximo);
					MPI_Isend(&buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
					esperar(ahora, 1);
					MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status_mpi);
					if (flag == 1) {
						MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request1);
					//	printf("recibo ack de: %d soy : %d\n", proximo, pid );
					} else {
						if (cont == 1 && proximo + 1 == pid){
							flag = 1;
						}
						if (proximo + 1 <= buffer[1] && es_ultimo == 0) {
							proximo = siguiente_pid(proximo, 0);
						}else{
						//para no pasarse de rango cuando ya llego al supuesto final del anillo salgo
							if (proximo + 1 > buffer[1] && es_ultimo == 0) {
								printf("mensaje a enviar: %d -> %d de: %d a %d \n", buffer[0], buffer[1], pid , proximo);
								cont = 1;
								proximo = 1;
								if (proximo == pid)	{
									flag = 1;
								}
							}
						}
						if (es_ultimo) {
							proximo = siguiente_pid(proximo, 0);
							es_ultimo = 0;
						}
					}

				}
			}
		} else {
			//todavia no termino
			if (buffer[1] < pid) {
				//sigue con un nuevo cl

				buffer[1] = pid;

			}
			flag = 0;
			while (flag == 0) { //aca tengo q esperar t segundos q no se como poner eso 
				printf("mensaje a enviar: %d -> %d de: %d a %d \n", buffer[0], buffer[1], pid , proximo);
				MPI_Isend(&buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
				esperar(ahora, 1);
				MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status_mpi);
				if (flag == 1) {
					MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request1);
				} else {
					if (cont == 1 && proximo + 1 == pid){
							flag = 1;
						}
					if (proximo + 1 <= buffer[1] && es_ultimo == 0) {
						proximo = siguiente_pid(proximo, 0);
					}else{
						if (proximo + 1 > buffer[1] && es_ultimo == 0) {
							printf("mensaje a enviar: %d -> %d de: %d a %d \n", buffer[0], buffer[1], pid , proximo);
								cont = 1;
								proximo = 1;
								if (proximo == pid)	{
									flag = 1;
								}
						}
					}
					if (es_ultimo) {
				//		printf("proximo: %d de %d \n",proximo, pid);
						proximo = siguiente_pid(proximo, 0);
						es_ultimo = 0;
					}
				}


			}



		}


		/* Actualizo valor de la hora. */

		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
}


