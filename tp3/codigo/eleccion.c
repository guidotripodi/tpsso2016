#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo)
{
 t_pid res= 0; /* Para silenciar el warning del compilador. */

 if (es_ultimo)
	res= 1;
 else
	res= pid+1;

 return res;
}

void iniciar_eleccion(t_pid pid, int es_ultimo)
{
 /* Completar acá el algoritmo de inicio de la elección.
  * Si no está bien documentado, no aprueba.
  HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
  HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
  */
	  char * mensaje[2];
	  mensaje[0]= pid;
	  mensaje[1] = pid;
	  proximo = siguiente_pid(pid, es_ultimo);
	  MPI_Isend(mensaje, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD);		
	  //aca lo q hago es crearme la tupla y mandarla iniciando la eleccion
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout)
{
 static t_status status= NO_LIDER;
 double ahora= MPI_Wtime();
 double tiempo_maximo= ahora+timeout;
 t_pid proximo= siguiente_pid(pid, es_ultimo);

 while (ahora<tiempo_maximo)
	{
	 /* Completar acá el algoritmo de elección de líder.
	  * Si no está bien documentado, no aprueba.
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!

LA IDEA ACA ES RECIBIR MSJ Y CHEQUEAR SI ID = CL Y SINO SEGUIR MANDANDO EL MSJ

          */
	
		MPI_Irecv(&buffer, 2, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
		origen = status.MPI_SOURCE;
		//envio una señal de ACK al que me envio el mjs para avisar que lo recibi

		MPI_Isend(NULL, 0, MPI_INT, origen, TAG_OTORGADO, COMM_WORLD);
		// esto no se si esta bien
		

		if (buffer[0] == pid){
			//hay lider
			if (buffer[1] == pid) {
				//soy lider cambio status no se cual 
				status = LIDER;
			}if (buffer[1] > pid){
				//el lider esta mas adelante
				//mando msj con cl cl
				buffer[0] = buffer[1];
				buffer[1] = buffer[1];
				MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD); 
				while(!llego){ //aca tengo q esperar t segundos q no se como poner eso 
					MPI_Irecv(&ck, 2, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
					//aca tengo q ver si recibi o no msj si no recibi mando al siguiente
					proximo = siguiente_pid(proximo,es_ultimo);
					MPI_Isend(buffer, 1, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD); 
				}
			}
		}else{
			//todavia no termino
			if (buffer[1] < pid){
				//sigue con un nuevo cl
				buffer[0] = pid;
				buffer[1] = pid;
				MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD); 
				while(!llego){ //aca tengo q esperar t segundos q no se como poner eso 
					MPI_Irecv(&ck, 2, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
					//aca tengo q ver si recibi o no msj si no recibi mando al siguiente
					proximo = siguiente_pid(proximo,es_ultimo);
					MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD); 
				}
			}

		}


	 /* Actualizo valor de la hora. */

	 ahora= MPI_Wtime();
	}

 /* Reporto mi status al final de la ronda. */
 printf("Proceso %u %s líder.\n", pid, (status==LIDER ? "es" : "no es"));
}
