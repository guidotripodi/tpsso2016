#include <vector>
#include <queue>
#include "sched_rr2.h"
#include "basesched.h"
#include <iostream>

using namespace std;

SchedRR2::SchedRR2(vector<int> argn) {
	// Round robin recibe la cantidad de cores y sus cpu_quantum por parámetro
	queue<int> cola;
	for (int i = 0; i < argn[0]; i++){
		quantum.push_back(argn[i+1]); //el quantum del core i es el parámetro i+1 (porque el primer param es #cores)
		quantumActual.push_back(argn[i + 1]);
		colas.push_back(cola);
		cantidadPorCore.push_back(0);

	}
}

SchedRR2::~SchedRR2() {

}


void SchedRR2::load(int pid) {
	int core = coreMenosProcesos();
	colas[core].push(pid);
	cantidadPorCore[core]++;
}

void SchedRR2::unblock(int pid) {
	int core = (bloqueados.find(pid))->second;
	bloqueados.erase(pid);
	colas[core].push(pid);
}

int SchedRR2::tick(int cpu, const enum Motivo m) {
	switch (m) {
		case EXIT:
			quantumActual[cpu] = quantum[cpu];
			cantidadPorCore[cpu]--;
			return next(cpu);
			break;
		case BLOCK:
			quantumActual[cpu] = quantum[cpu];
			bloqueados.insert(pair<int,int> (current_pid(cpu),cpu));
			return next(cpu);
			break;
		case TICK:
			if (current_pid(cpu) == IDLE_TASK) {
				if (!colas[cpu].empty()) {
					quantumActual[cpu] = quantum[cpu];
					return next(cpu);
				} else {
					return IDLE_TASK;
				}
			} else {
				quantumActual[cpu]--;
				if (quantumActual[cpu] == 0) {
					colas[cpu].push(current_pid(cpu));
					quantumActual[cpu] = quantum[cpu];
					return next(cpu);
				} else {
					return current_pid(cpu);

				}
			}
			break;
	}
}
	

int SchedRR2::next(int cpu) {
	int pid ;
	if (colas[cpu].empty()){ 
		pid= IDLE_TASK;  //no hay mas tareas -.-> idle_task
	}else {
		pid = colas[cpu].front();	//saco la tarea de la cola
		colas[cpu].pop();
	} 
	quantumActual[cpu] = quantum[cpu];	 //no hubo tick, recien se asigno al core
	return pid;
}

int SchedRR2::coreMenosProcesos(){
	int minimo = cantidadPorCore[0];
	int coreMinimo = 0;
	for (int i = 1; i < (int) cantidadPorCore.size(); ++i){
		if (minimo > cantidadPorCore[i]){
			minimo = cantidadPorCore[i];
			coreMinimo = i;
		}
	}
	return coreMinimo;
}