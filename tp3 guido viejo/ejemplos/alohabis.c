#include <stdio.h>
#include "mpi.h"

#define RANK_CERO 0

int main(int argc, char *argv[])
{
    int mi_rank;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mi_rank);

    if(mi_rank == RANK_CERO)
        printf("¡Aloha honua! Soy el proceso con menor rank.\n");
    else
        printf("¡Aloha honua! Soy uno más del montón.\n");
    
    MPI_Finalize();
    return 0;
}
