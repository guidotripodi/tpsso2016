#ifndef __SCHED_NOMIS__
#define __SCHED_NOMIS__

#include <vector>
#include <queue>
#include <map>
#include "basesched.h"


class SchedNoMistery : public SchedBase {
  public:
    SchedNoMistery(std::vector<int> argn);
    ~SchedNoMistery();
    virtual void load(int pid);
    virtual void unblock(int pid);
    virtual int tick(int cpu, const enum Motivo m);  

  private:
    int next(int cpu);
    int contQuantumPasados;
    std::queue<int> q;
    int quantumActual;
   // std::queue<int> bloqueados;
    //std::vector< std::queue< int > > quantumDeTareas; //lista con los quantums de cada PID
    std::queue<int> desbloqueados;
    std::vector<int> quantum;
   // std::vector<int> quantumRestantes;
    std::map<int,int> pid_quantum;
    int pidInicial;


};

#endif
