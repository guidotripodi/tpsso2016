#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo) {
	t_pid res = 0; /* Para silenciar el warning del compilador. */
	printf("para %d - el ultimo es %d\n", pid, es_ultimo);
	if (pid == es_ultimo)
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
	printf("arranque y soy: %d\n", pid);
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
	int buffer[2];
	MPI_Status status_mpi;
	MPI_Request request;
	MPI_Request request1;
	buffer[0] = buffer[1] = 1;
	printf("@@ %u\n", timeout);
	while (ahora < tiempo_maximo) {

		flag = 0;
		while (flag == 0 && MPI_Wtime() < tiempo_maximo) {
			//aca no iria proximo sino el anterior a el 
			MPI_Iprobe(ANY_SOURCE, TAG_OTORGADO, MPI_COMM_WORLD, &flag, &status_mpi);
		}

		//		printf("valor flag ?  %d\n", flag);
		//if(flag == 0) break;
		MPI_Irecv(&buffer, 2, MPI_INT, ANY_SOURCE, TAG_OTORGADO, COMM_WORLD, &request);

		//		printf("buffer[0] %d\n",buffer[0]);
		//		printf("pid %d\n",pid);
		//ACA HAY Q AVERIGUAR QUIEN ES EL ORIGEN COMO FUNCIONA REQUEST 
		origen = status_mpi.MPI_SOURCE;
		//		printf("origen: %d \n",origen );
		//envio una se�al de ACK al que me envio el mjs para avisar que lo recibi
		printf("ack: %d -> %d \n", pid, origen);
		MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request1);
		// esto no se si esta bien


		if (buffer[0] == pid) {
			//			printf("soy el origen!! %d\n", pid );
			//hay lider
			if (buffer[1] == pid) {
				//soy lider cambio status no se cual 

				status = LIDER;
			}
			if (buffer[1] > pid) {
				//el lider esta mas adelante
				//mando msj con cl cl

				buffer[0] = buffer[1];
				buffer[1] = buffer[1];
				flag = 0;


				while (flag == 0) { //aca tengo q esperar t segundos q no se como poner eso 
					printf("el lider es otro: %d -> %d \n", pid, proximo);
					MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
					esperar(1);
					MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status_mpi);
					if (flag == 1) {
						MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request1);
					} else {
						proximo = siguiente_pid(proximo, es_ultimo);
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
				printf("token ajeno: %d -> %d token: {%d,%d}\n", pid, proximo, buffer[0], buffer[1]);
				MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request);
				esperar(1);
				MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status_mpi);
				if (flag == 1) {
					MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request1);
				} else {
					proximo = siguiente_pid(proximo, es_ultimo);
				}

			}



		}


		/* Actualizo valor de la hora. */

		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status == LIDER ? "es" : "no es"));
}


/*		mientras(tenga que existir){
		reciboUnMensaje();
		si (tokenesmio ){
			si(token.cl > yo){
				enviar(token{cl, cl})
			}sino{
				yo soy el lider muajajaja
			}
		}sino{ //el token no lo cree

			si(cl < yo) {
				nuevocl = yo; 
				
			}sino{
				nuevocl = cl;
				
			}
			enviar(token{i, nuevocl});
		}
		}
 */
/* Completar ac� el algoritmo de elecci�n de l�der.
 * Si no est� bien documentado, no aprueba.
	HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
	HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
	HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!

LA IDEA ACA ES RECIBIR MSJ Y CHEQUEAR SI ID = CL Y SINO SEGUIR MANDANDO EL MSJ

 */
