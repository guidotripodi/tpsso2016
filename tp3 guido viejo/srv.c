#include "srv.h"

/*
 *  Ejemplo de servidor que tiene el "sí fácil" para con su
 *  cliente y no se lleva bien con los demás servidores.
 *
 */
/*Por medio del paper An optimal algorithm for mutual exclusion in computer networks implementamos el 
algoritmo correspondiente, basandonos en la implemntacion del cliente y servidor otorgada, modificamos
la del servidor correspondiendonos al algoritmo del paper enunciado */



void servidor(int mi_cliente)

{
    MPI_Status status; int origen, tag;
    int hay_pedido_local = FALSE;
    int listo_para_salir = FALSE;
    int n_ranks, mi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    /*VARIABLES DECLARADAS PARA CONEXION SRV SRV*/
    int tengo_seccion_critica = FALSE; // Indica cuando el cliente esta en la seccion critica
    int mi_secuencia; //indica el valor de secuencia que se enviara para tener un order de prioridad
    int buffer;
    int secuencia_maxima = 0; //sera el valor del ultimo pedido de servidor que se realizo
    int reply_faltantes; // la cantidad de reply que me faltan para poder otorgar la seccion critica
    int clientes_activos =  n_ranks/2;
    int faltan_responder[n_ranks/2]; //son los servidores a los que me faltan dar señal de reply

    /*INICIALIZACION DE VARIABLES*/
    mi_rank = mi_rank/2; //Esta se divide por dos ya que los servidores son solo los pares
    n_ranks = n_ranks/2; //Esta se divide por dos ya que los servidores son solo los pares
    int i;
    for (i = 0; i < n_ranks; i++){
        faltan_responder[i] = FALSE;
    }

    while( ! listo_para_salir ) {
        MPI_Recv(&buffer, 1, MPI_INT, ANY_SOURCE, ANY_TAG, COMM_WORLD, &status);
        origen = status.MPI_SOURCE; //Es el rank que recibi
        tag = status.MPI_TAG;

        /*CONEXION SRV CLI*/
        if (tag == TAG_PEDIDO) {
            //Me llega un pedido de mi cliente para entrar a la seccion critica
            assert(origen == mi_cliente);
            assert(tengo_seccion_critica == FALSE);
            assert(hay_pedido_local == FALSE);
            debug("Mi cliente solicita acceso exclusivo");
            hay_pedido_local = TRUE;
            mi_secuencia = secuencia_maxima + 1;
            reply_faltantes = n_ranks - 1;

            /*Una vez que el cliente solicita un pedido para uso de seccion critica se chequea si es el
            unico cli-srv activo o no, si es el unico se le otorga permiso, si no lo es se envia una señal de
            req a todos los servidores con el numero de secuencia */
            if (n_ranks == 1) {
                //solo hay uno
                debug("Dándole permiso");
                tengo_seccion_critica = TRUE;
                MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            } else {
                //hay mas de uno. Envio request a los demas con tag request.
                for (i = 0; i < n_ranks; i++) {
                    if (i != mi_rank){
                        debug("Pido acceso a todos");
                        MPI_Send(&mi_secuencia, 1, MPI_INT, i*2, TAG_REQUEST, COMM_WORLD);
                    }
                }
            }
        } else if (tag == TAG_LIBERO) {

            /*Seccion cuando se liberan los recursos del cliente y se envia señales reply a todos
            los servidores que solicitaron uso exclusivo */
            assert(origen == mi_cliente);
            assert(hay_pedido_local == TRUE);
            assert(tengo_seccion_critica == TRUE);
            hay_pedido_local = FALSE;
            tengo_seccion_critica = FALSE;
            debug("Mi cliente libera su acceso exclusivo");
            for (i = 0; i < n_ranks; i++){
                if ( faltan_responder[i] == TRUE ) {
                    debug("Respondo pedidos guardados");
                    MPI_Send(NULL, 0, MPI_INT, i*2, TAG_REPLY, COMM_WORLD);
                    faltan_responder[i] = FALSE;
                }
            }
        } else if (tag == TAG_TERMINE) {
            /* Cuando finaliza el cliente se envia una señal de SIN_CLIENTE a los demas
            servidores para que disminuyan su variable clientes_activos*/
            assert(origen == mi_cliente);
            assert(hay_pedido_local == FALSE);
            assert(tengo_seccion_critica == FALSE);
            clientes_activos--;
            debug("Mi cliente avisa que terminó");
            for (i = 0; i < n_ranks; i++){
                if (i != mi_rank) {
                    MPI_Send(NULL, 0, MPI_INT, i*2, TAG_SIN_CLIENTE, COMM_WORLD);
                }
            }
        }

        /*CONEXION SRV SRV*/
        if (tag == TAG_REQUEST) {
            /*Si se recibe un tag del tipo request se chequea el valor de la secuencia,
            y dependiendo si el servidor que recibio el req tiene el lock activo otorga o no
            el pedido. En caso de no otorgarlo por tener hay_pedido_local activo, se
            chequea los valores de la secuencia y rank respectivo para ver la prioridad,
            en caso de tener mayor prioridad se le otorgara por mas que tenga un pedido activo*/
            if(buffer > secuencia_maxima) secuencia_maxima = buffer;
            debug("Me pidieron el recurso");


            if (tengo_seccion_critica == TRUE){
                //tengo la seccion critica, nadie me la sacara!
                debug("Tengo el lock, tiene que esperar, guardo el pedido");
                assert(faltan_responder[origen/2] == FALSE);
                faltan_responder[origen/2] = TRUE; // le respondo despues
            } else if(!hay_pedido_local) {
                debug("Lo otorgo por que no lo necesito");
                MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);
            } else {
                if (buffer < mi_secuencia || (buffer == mi_secuencia && (origen/2) < mi_rank) ) {
                    //Si mi secuencia es mas grande o , si son iguales y tengo rank mayor, le mando reply 
                    debug("Lo otorgo por que tiene mayor prioridad");
                    MPI_Send(NULL, 0, MPI_INT, origen, TAG_REPLY, COMM_WORLD);
                } else {
                    //tengo mayor prioridad, que espere!
                    debug("Tiene menor prioridad, no lo otorgo y guardo el pedido");
                    assert(faltan_responder[origen/2] == FALSE);
                    faltan_responder[origen/2] = TRUE;
                }
            }
        } else if (tag == TAG_REPLY) {

            /*Si se recibe un reply, se chequea la cantidad que ya se obtuvieron, en caso de
            tener todos los reply de los srv se otorga el permiso al cliente*/
            assert(tengo_seccion_critica == FALSE);
            assert(hay_pedido_local == TRUE);
            reply_faltantes --; //uno menos...
            if (reply_faltantes == 0) {
                //al fin me respondieron todos! puedo otorgar seccion critica
                tengo_seccion_critica = TRUE;
                debug("Dándole permiso");
                //envio a mi cliente el permiso para acceder
                MPI_Send(NULL, 0, MPI_INT, mi_cliente, TAG_OTORGADO, COMM_WORLD);
            }
        } else if (tag == TAG_SIN_CLIENTE) {
            //se fue algun cliente de algun server, disminuyo la cantidad de clientes activos
            clientes_activos--;
        }

        if (clientes_activos == 0) {
            //A ningun server le queda cliente, cierro el servidor.  
            assert(listo_para_salir == FALSE);
            listo_para_salir = TRUE;
        }
    }
}
