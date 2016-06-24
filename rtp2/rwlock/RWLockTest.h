#define RWLockTest_h
#include <stdio.h>

    
    int main(int argc, char* argv[]);
    void *soy_lector(void *p_numero);
    void *soy_escritor(void *p_numero);
    int NUM_THREADS;
    int variable;
    RWLock para_variable;
    
        
        


