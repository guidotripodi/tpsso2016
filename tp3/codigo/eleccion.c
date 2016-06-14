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
	  int mensaje[2];
	  mensaje[0]= pid;
	  mensaje[1] = pid;
	  MPI_Request request;
	  int proximo = siguiente_pid(pid, es_ultimo);
	  MPI_Isend(mensaje, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD,request);		
	  //aca lo q hago es crearme la tupla y mandarla iniciando la eleccion
}

void esperar(int segundos){
	int actual = MPI_Wtime();
	while(MPI_Wtime() <= actual + segundos){}
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout)
{
 static t_status status_lider = NO_LIDER;
 double ahora= MPI_Wtime();
 double tiempo_maximo= ahora+timeout;
 t_pid proximo= siguiente_pid(pid, es_ultimo);
 int flag, origen;
 int buffer[2];
 MPI_Status status;
 MPI_Request request;


 while (ahora<tiempo_maximo)
	{

		flag = 0;
		while(flag == 0 && MPI_Wtime()<tiempo_maximo){
			MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status);
		}
		if(flag == 0) break;
		MPI_Irecv(&buffer, 2, MPI_INT, ANY_SOURCE, TAG_OTORGADO, COMM_WORLD, &request);
		
		origen = status.MPI_SOURCE;
		//envio una señal de ACK al que me envio el mjs para avisar que lo recibi

		MPI_Isend(NULL, 0, MPI_INT, origen, TAG_ACK, COMM_WORLD, &request);
		// esto no se si esta bien
		

		if (buffer[0] == pid){
			//hay lider
			if (buffer[1] == pid) {
				//soy lider cambio status no se cual 
				status_lider = LIDER;
			}
			if (buffer[1] > pid){
				//el lider esta mas adelante
				//mando msj con cl cl
				buffer[0] = buffer[1];
				buffer[1] = buffer[1];
				flag = 0;

				
				while(flag == 0){ //aca tengo q esperar t segundos q no se como poner eso 
					
					MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request); 
					esperar(1);
					MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status);
					if (flag == 1){
						MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request);
					}else{
						proximo = siguiente_pid(proximo,es_ultimo); 
					}
					
				}
			}
		}else{
			//todavia no termino
			if (buffer[1] < pid){
				//sigue con un nuevo cl
				
				buffer[1] = pid;
				 
			}	
			flag = 0;
			while(flag == 0){ //aca tengo q esperar t segundos q no se como poner eso 

					MPI_Isend(buffer, 2, MPI_INT, proximo, TAG_OTORGADO, COMM_WORLD, &request); 
					esperar(1);
					MPI_Iprobe(proximo, TAG_ACK, MPI_COMM_WORLD, &flag, &status);
					if (flag == 1){
						MPI_Irecv(NULL, 0, MPI_INT, proximo, TAG_ACK, COMM_WORLD, &request);
					}else{
						proximo = siguiente_pid(proximo,es_ultimo); 
					}
					
			}
			
		

		}


	 /* Actualizo valor de la hora. */

	 ahora= MPI_Wtime();
	}

 /* Reporto mi status al final de la ronda. */
 printf("Proceso %u %s líder.\n", pid, (status_lider==LIDER ? "es" : "no es"));
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
	 /* Completar acá el algoritmo de elección de líder.
	  * Si no está bien documentado, no aprueba.
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!
         HAY Q RESOLVER TODO ACA DENTRO SIN TOCAR NADA DE AFUERA!

LA IDEA ACA ES RECIBIR MSJ Y CHEQUEAR SI ID = CL Y SINO SEGUIR MANDANDO EL MSJ

          */
