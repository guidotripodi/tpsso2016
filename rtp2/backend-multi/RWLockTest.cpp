#include "RWLock.h"
#include "RWLockTest.h"
#include <unistd.h>



int main(int argc, char* argv[]) {
	NUM_THREADS = 50;
	variable = 0;
	pthread_t threads[NUM_THREADS];
	int i;
	int array[NUM_THREADS];
	for (int h = 0; h < NUM_THREADS; ++h)
	{
		array[h] = h;
	}
	printf("El valor de la variable al empezar es: %d\n",variable);
	for (i = 0; i < NUM_THREADS; ++i){
		if(!(i % 2)){
			pthread_create(&threads[i], NULL, soy_escritor, &array[i]);
		}else{
			pthread_create(&threads[i], NULL, soy_lector, &array[i]);
		}
	}

	for (int j = 0; j < NUM_THREADS; ++j){
		pthread_join(threads[j], NULL);
	}
	pthread_exit(NULL);
    return 0;
}


void *soy_lector(void *p_numero){
	int mi_numero = *((int *) p_numero);
	for (int j = 0; j < 1; ++j){
		para_variable.rlock();
		for (int i = 0; i < 5; ++i){
		printf("Lector numero: %d  ", mi_numero);
		printf(" Leo valor %d cantidad de veces que tengo el lock sin liberarlo: %d \n\n", variable, i );
		sleep(10);
		}
		//NO SE SI TENDRIA Q METER EL UNLOCK PARA VER LO DE Q HAYA MAS DE UN LECTOR 
		para_variable.runlock();
	}
	pthread_exit(NULL);
	return NULL;
}

void *soy_escritor(void *p_numero){
	int mi_numero = *((int *) p_numero);
	for (int j = 0; j < 1; ++j){
		para_variable.wlock();
		for (int i = 0; i < 5; ++i){
		variable++;
		printf("Escritor numero: %d  ", mi_numero);
		printf(" cambio valor %d, cantidad de veces que tengo el lock sin liberarlo: %d \n\n", variable, i );
		sleep(2);
		}
		para_variable.wunlock();
	}
	pthread_exit(NULL);
	return NULL;

}