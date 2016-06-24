#ifndef RWLock_h
#define RWLock_h
#include <iostream>

class RWLock {
    public:
        RWLock();
        ~RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
        pthread_mutex_t turnstile;
        pthread_cond_t room_empty;
        pthread_mutex_t readers_mutex;
        unsigned int readers;
        pthread_rwlock_t rwlock;
};

#endif
