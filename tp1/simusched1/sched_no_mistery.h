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
    int quantumActual;
    int pidInicial;
    std::vector<int> quantum;
    std::queue<int> q;
    std::queue<int> desbloqueados;
    std::map<int,int> pid_quantum;
    std::map<int,int> pid_Tarde;
    std::map<int,int> pid_Bloqueado;


};

#endif
