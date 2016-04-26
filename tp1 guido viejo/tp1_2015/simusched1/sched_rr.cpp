#include <vector>
#include <queue>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;


SchedRR::SchedRR(vector<int> argn) {
	// Round robin recibe la cantidad de cores y sus cpu_quantum por parámetro
	for (int i = 0; i < argn[0]; i++){
		cores.push_back(-1); //todos los cores arrancan vacíos
		quantum.push_back(argn[i+1]); //el quantum del core i es el parámetro i+1 (porque el primer param es #cores)
		quantumActual.push_back(argn[i + 1]);
	}
}

SchedRR::~SchedRR() {

}


void SchedRR::load(int pid) {
	q.push(pid); // llegó una tarea nueva
}

void SchedRR::unblock(int pid) {
	bloqueados.remove(pid);
	q.push(pid);
}

int SchedRR::tick(int cpu, const enum Motivo m) {
	int sig;

	switch (m) {
		case EXIT:
			quantumActual[cpu] = quantum[cpu];
			return next(cpu);
			break;
		case BLOCK:
			quantumActual[cpu] = quantum[cpu];
			bloqueados.push_back(current_pid(cpu));
			return next(cpu);
			break;
		case TICK:
			if (current_pid(cpu) == IDLE_TASK) {
				if (!q.empty()) {
					sig = q.front(); q.pop();
					quantumActual[cpu] = quantum[cpu];
					return sig;
				} else {
					return IDLE_TASK;
				}
			} else {
				quantumActual[cpu]--;
				if (quantumActual[cpu] == 0) {
					q.push(current_pid(cpu));
					
					sig = q.front(); q.pop();
					quantumActual[cpu] = quantum[cpu];
					return sig;
				} else {
					return current_pid(cpu);

				}
			}
			break;
	}

	
}

int SchedRR::next(int cpu){
	int pid ;
	if (q.empty()){ 
		pid= IDLE_TASK;  //no hay mas tareas -.-> idle_task
	}else {
		pid = q.front();	//saco la tarea de la cola
		q.pop();
	} 
	
	cores[cpu] = pid;
	quantumActual[cpu] = quantum[cpu];	 //no hubo tick, recien se asigno al core
	return pid;
}