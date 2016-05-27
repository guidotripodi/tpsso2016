#include "RWLock.h"

RWLock :: RWLock() {
	pthread_mutex_init(&readers_mutex, NULL);
	pthread_mutex_init(&turnstile, NULL);
	pthread_cond_init(&room_empty, NULL);
	readers = 0;
}

RWLock :: ~RWLock() {
	pthread_mutex_destroy(&readers_mutex);
	pthread_mutex_destroy(&turnstile);
	pthread_cond_destroy(&room_empty);
}

void RWLock :: rlock() {
	pthread_mutex_lock(&turnstile);
	pthread_mutex_unlock(&turnstile);

	pthread_mutex_lock(&readers_mutex);
	readers++;
	pthread_mutex_unlock(&readers_mutex);
}

void RWLock :: wlock() {
	pthread_mutex_lock(&turnstile);

	pthread_mutex_lock(&readers_mutex);
	while(readers != 0)
		pthread_cond_wait(&room_empty, &readers_mutex);
	pthread_mutex_unlock(&readers_mutex);
}

void RWLock :: runlock() {
	pthread_mutex_lock(&readers_mutex);
	readers--;
	if (readers == 0) {
		pthread_cond_signal(&room_empty);		
	}
	pthread_mutex_unlock(&readers_mutex);	
}

void RWLock :: wunlock() {
	pthread_mutex_unlock(&turnstile);
}

