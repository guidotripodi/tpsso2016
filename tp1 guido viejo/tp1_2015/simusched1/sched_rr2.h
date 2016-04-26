#ifndef __SCHED_RR2__
#define __SCHED_RR2__

#include <vector>
#include <queue>
#include <list>
#include <map>
#include "basesched.h"

class SchedRR2 : public SchedBase {
	public:
		SchedRR2(std::vector<int> argn);
        	~SchedRR2();
		virtual void initialize() {};
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(int cpu, const enum Motivo m);

	private:
		std::vector<int> quantum;
		std::vector<int> quantumActual;
		std::vector<std::queue<int> > colas;
		std::vector<int> cantidadPorCore;
		std::map<int,int> bloqueados; // key : tarea, sign: core
		int coreMenosProcesos();
		int next(int cpu);
};

#endif
