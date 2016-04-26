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
    bool sinTareas();
   // int contQuantumPasados;
    std::vector< std::queue<int> > quantumDeTareas; // las tareas que tienen que correr cada quantum
    std::vector<int> quantum; // quantums por parametro
    std::queue<int> tareaEntroTarde; // tarea que entro despues del tiempo 0
    
    int quantumActual; // que quantum hay que correr ahora

    int quantumRestante; // quantum restante de la tarea actual
    int quantumTarea; // quantum total de la tarea actual

    std::queue<int> bloqueados;
   


};

#endif
