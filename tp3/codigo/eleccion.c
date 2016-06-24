#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"
//#include <list>


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
	int proximo_aux = 1;
	int flag, origen, tag, en_espera;
	int token[2];
	MPI_Status status_mpi;
	MPI_Request request[6];
	en_espera = 0;

	token[0] = token[1] = 1;

	if (status == LIDER){
		//limpio status por si ya fue usado antes y quedo con LIDER
		status = NO_LIDER;
	}

	while (ahora < tiempo_maximo) {

		flag = 0;
		MPI_Iprobe(ANY_SOURCE, ANY_TAG, MPI_COMM_WORLD, &flag, &status_mpi);

		if (flag == 1) {
			origen = status_mpi.MPI_SOURCE;
			tag = status_mpi.MPI_TAG;
			
			if (tag == TAG_OTORGADO && en_espera == 0) {
			
				MPI_Irecv(&token, 2, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &request[0]);
				printf("ack: %d -> %d \n", pid, origen);
				MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request[1]);
			
				if (token[0] == pid) {
					if (token[1] == pid) {
						//soy lider cambio status no se cual 
						status = LIDER;
						printf("que pid gano: %d\n", pid);
					}
					if (token[1] > pid) {
						//el lider esta mas adelante
						//mando msj con cl cl
						token[0] = token[1];

						printf("el lider es otro: %d -> %d  token: {%d,%d}\n", pid, proximo, token[0], token[1]);
						//me guardo el ultimo proximo antes de arrancar la rueda
						proximo_aux = proximo;
						MPI_Isend(token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request[2]);
						esperar(2);
						en_espera = 1;
					}
				}else {
					//todavia no termino
					if (token[1] < pid) {
						//sigue con un nuevo cl
						token[1] = pid;
					}
					printf("token ajeno: %d -> %d token: {%d,%d}\n", pid, proximo, token[0], token[1]);
					proximo_aux = proximo;
					MPI_Isend(&token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request[3]);
					esperar(2);
					en_espera = 1;
				}
				continue;
			}
		
			if (tag == TAG_ACK) {

				//recibi ack me fijo si estaba o no en la rueda
				MPI_Irecv(NULL, 0, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &request[4]);
				if (en_espera == 1)	{
					en_espera = 0;
					//actualizo al valor real proximo este hay q chequear el limite
					proximo = siguiente_pid(proximo_aux,0);
				}
			}
		}else{
			if (ahora != MPI_Wtime()){
				//revisarSiHayQueReenviar();
			}
		
		//si estoy en espera y no hay flag true voy al proximo del anillo siempre que sea <= al cl
			if (en_espera == 1)	{
				if (proximo + 1 <= token[1]){
					proximo = siguiente_pid(proximo,0);
					MPI_Isend(&token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request[5]);
					printf("el pid: %d envia token {%d, %d} al proximo: %d \n",pid, token[0],token[1], proximo);
					esperar(2);
				}else{
					proximo = 1;
					MPI_Isend(&token, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request[5]);
					printf("el pid: %d envia token {%d, %d} al proximo: %d \n",pid, token[0],token[1], proximo);
					esperar(2);
				}
				//si pegue la vuelta y nadie me respondio salgo
				if (proximo + 1 == pid)	{
					en_espera = 0;
					proximo = siguiente_pid(proximo_aux,0);	
				
				}
			}
		}
		/* Actualizo valor de la hora. */
		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
}


