#include "RWLock.h"
#include "RWLockTest.h"



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

	for (int j = 0; j < 5; ++j){
		para_variable.rlock();
		printf("Lector numero: %d  ", mi_numero);
		printf(" Leo valor %d\n", variable );
		para_variable.runlock();
	}
	pthread_exit(NULL);
	return NULL;
}

void *soy_escritor(void *p_numero){
	int mi_numero = *((int *) p_numero);
	for (int j = 0; j < 5; ++j){
		para_variable.wlock();
		variable++;
		printf("Escritor numero: %d  ", mi_numero);
		printf(" cambio valor %d\n", variable );
		para_variable.wunlock();
	}
	pthread_exit(NULL);
	return NULL;

}