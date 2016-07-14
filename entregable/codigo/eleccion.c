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
	 */
	int mensaje[2];
	int destino = 0;
	mensaje[0] = pid;
	mensaje[1] = pid;
	MPI_Request request;
	//MPI_Status status_mpi;
	int proximo = siguiente_pid(pid, es_ultimo);
	int flag;
	destino = proximo;
	while (destino > 0) {
		if (destino == pid) //si pegue la vuelta y no hay nadie salgo
			break;
			MPI_Isend(mensaje, 2, MPI_INT, destino, TAG_OTORGADO, COMM_WORLD, &request);
			flag = esperar(1);
			//esperar obtengo el valor del flag en caso de recibir o no ack
			if (flag == 0) {
				destino++;
			} else {
				MPI_Irecv(NULL, 0, MPI_INT, ANY_SOURCE, TAG_ACK, COMM_WORLD, &request);
				destino = 0;
			}
					//Si flag es 0 tengo que enviarle al proximo sino recibo el ack y salgo del ciclo
	}
	//aca lo q hago es crearme la tupla y mandarla iniciando la eleccion
}

int esperar( int segundos) {
	int actual = MPI_Wtime();
	int flag = 0;
	MPI_Status status;
	while (flag == 0 && MPI_Wtime() <= actual + segundos) {
		MPI_Iprobe(ANY_SOURCE, TAG_ACK, MPI_COMM_WORLD, &flag, &status); // si alguien me manda un ack es porque le mande un token y considero que lo envie bien
	}
	return flag;
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout) {
	static t_status status = NO_LIDER;
	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + timeout;
	int proximo = siguiente_pid(pid, es_ultimo);
	int flag, origen, tag;
	int token[2];
	int n_ranks;
	MPI_Status status_mpi;
	MPI_Request request;
	MPI_Request request1;
	MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

	int destino;
	int candidatoAUltimo; // no quiero enviarle un mensaje a alguien fuera de rango
	if (es_ultimo == 1) {
		candidatoAUltimo = pid; // si soy el ultimo soy el mejor candidato a ultimo 
	} else {
		candidatoAUltimo = proximo; // seguro el proximo esta
	}
	if (status == LIDER){
		status = 0;
		status = NO_LIDER;
	}

	token[0] = token[1] = 1;

	while (ahora < tiempo_maximo) {

		flag = 0;
		MPI_Iprobe(ANY_SOURCE, ANY_TAG, MPI_COMM_WORLD, &flag, &status_mpi);
		
		if (flag == 1) {
			origen = status_mpi.MPI_SOURCE;
			tag = status_mpi.MPI_TAG;
			if (candidatoAUltimo < origen) candidatoAUltimo = origen; // aca me entero que hay alguien mas alla y se convierte en mi nuevo candidato
			// recibo any_tag si es un ack es un mensaje de alguien que me respondio tarde y lo descarto
			// si es un token lo proceso
			if (tag == TAG_OTORGADO) {
				MPI_Irecv(&token, 2, MPI_INT, ANY_SOURCE, TAG_OTORGADO, COMM_WORLD, &request);
				MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request1);
				
				if (candidatoAUltimo < token[1]) candidatoAUltimo = token[1]; // aca me entero que hay alguien mas alla y se convierte en mi nuevo candidato
				if (token[0] == pid) {
					//El token dio vuelta al anillo
					if (token[1] == pid) {
						//soy lider cambio status a LIDER
						status = LIDER;
					}
					if (token[1] > pid) {

						//el lider esta mas adelante
						//mando msj con cl cl
						status = NO_LIDER;
						token[0] = token[1];

					}
				} else {
					//todavia no termino la vuelta en el anillo
					if (token[1] < pid || es_ultimo) {
						//sigue con un nuevo cl
						token[1] = pid;
					}
				}

				if (proximo > n_ranks-1)	{
					proximo = 1;
				}
				destino = proximo;
				// reenvio el mensaje hasta que alguien me lo reciba, o se acabe mi tiempo de vida
				while (destino > 0) {
				printf("destino %d pid: %d\n", destino, pid );
					if (destino == pid) break;
					printf("enviando: %d -> %d {%d,%d}\n", pid, destino, token[0], token[1]);
					MPI_Isend(&token, 2, MPI_INT, destino, TAG_OTORGADO, COMM_WORLD, &request);
					flag = esperar(1);
					//envio mensaje y espero 1 segundos en la funcion 
					//esperar obtengo el valor del flag en caso de recibir o no ack

					if (flag == 0) {
						if (n_ranks-1 > destino)	{
							destino++;
						}else{
							destino = 1;
						}
					} else {
						MPI_Irecv(NULL, 0, MPI_INT, ANY_SOURCE, TAG_ACK, COMM_WORLD, &request);
						destino = 0;
					}
					//Si flag es 0 tengo que enviarle al proximo sino recibo el ack y salgo del ciclo
				}

			}
			if (tag == TAG_ACK)	{
				MPI_Irecv(NULL, 0, MPI_INT, ANY_SOURCE, TAG_ACK, COMM_WORLD, &request);
				//printf("recibi ack: %d -> %d  \n", origen, pid);
			}


		}


		/* Actualizo valor de la hora. */
		if (!(es_ultimo == 1 && status == NO_LIDER)){
			//si soy el ultimo y todavia no giro mi token completo no termino espero
			ahora = MPI_Wtime();
		}
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
	
	//vacio la cola de mensajes si me queda alguno
	/*int i =0;
	while(i < candidatoAUltimo){
		MPI_Irecv(NULL, 0, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &request);
		i++;
	}*/
}


