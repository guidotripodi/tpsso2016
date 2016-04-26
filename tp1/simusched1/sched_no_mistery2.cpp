#include <vector>
#include <queue>
#include "sched_no_mistery.h"
#include "basesched.h"
#include <iostream>
#include <stdio.h>

using namespace std;


SchedNoMistery::SchedNoMistery(vector<int> argn) {

	// Mistery recibe la cantidad de quantum por parámetro
	contQuantumPasados = 0;
	quantum.push_back(1);
	for (int i = 0; i < argn.size() ; i++){
		quantum.push_back(argn[i+1]);

	}

}


SchedNoMistery::~SchedNoMistery() {
}


void SchedNoMistery::load(int pid) {

	
	if (contQuantumPasados != 0){
		
		int primerQuantum = 0; // Si ya corrio algun quantum se lo contabiliza para ser sumado a la nueva tarea ingresada
		for (int i = 0; i < contQuantumPasados; ++i){
			if (i > quantum.size()){
				primerQuantum = primerQuantum + quantum.back(); // Si el contador es mayor a la lista de quantum sumo siempre el ultimo
		
			}else{
				primerQuantum = primerQuantum + quantum[i];
		
			}
		}
		pid_quantum.insert(pair<int, int> (pid,primerQuantum));

		int valorCola = q.size();
		std::queue<int> colaAux;
		colaAux.push(pid);
		for (int i = 0; i < valorCola; ++i)	{
			colaAux.push(q.front());
			q.pop();
		}
		for (int i = 0; i < valorCola+1; ++i)	{
			q.push(colaAux.front());
			colaAux.pop();
		}
		
	}else{
		pid_quantum.insert(pair<int, int> (pid, quantum[0]));
		q.push(pid); // llegó una tarea nueva
	}	
}

void SchedNoMistery::unblock(int pid) {
	//bloqueados.remove(pid);
	desbloqueados.push(pid);
}

int SchedNoMistery::tick(int cpu, const enum Motivo m) {
	int sig;

	switch (m) {
		case EXIT:
			return next(cpu);
			break;
		case BLOCK:
			return next(cpu);
			break;
		case TICK:
			if (current_pid(cpu) == IDLE_TASK) {
				if (!q.empty()) {
					sig = q.front(); q.pop();
					pidInicial = sig;
					quantumActual = (pid_quantum.find(sig))->second;
					return sig;
				} else {
					return IDLE_TASK;
				}
			} else {
				quantumActual--;
				if (quantumActual == 0) {
					q.push(current_pid(cpu));
					if (q.front() == pidInicial) {
						if (contQuantumPasados < quantum.size()-1)	{
							contQuantumPasados++;
						}
						if (contQuantumPasados < quantum.size()-1){
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = quantum.back();
						}
					}else{

						//ACA ME FIJO SI EL Q VA A VENIR ES UNO Q INGRESO DSP, SI YA INGRESO UNA VES VA CON EL QUANTUM COMUN
						//POR ESO DSP LO ELIMINO Y LE MANDO QUANTUM SUB CERO GENERAL
						if ((pid_quantum.find(q.front()))->second == quantum[0]) {
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = (pid_quantum.find(q.front()))->second;
							pid_quantum.erase(q.front());
							pid_quantum.insert(pair<int, int> (q.front(), quantum[0]));
						}
					}
					sig = q.front(); q.pop();
					return sig;
				} else {
					return current_pid(cpu);

				}
			}
			break;
	}

	
}

int SchedNoMistery::next(int cpu){
	int pid ;
	if (q.empty()){ 
		pid= IDLE_TASK;  //no hay mas tareas -.-> idle_task
	}else {
		//quantumActual = quantumDeTareas[q.front()].front();
		if (current_pid(cpu) == pidInicial)	{
			if (contQuantumPasados < quantum.size()-1)	{
					contQuantumPasados++;
			}
		 	pidInicial = q.front();
		}
		pid = q.front();	//saco la tarea de la cola
		if ((pid_quantum.find(pid))->second == quantum[0]) {
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = (pid_quantum.find(pid))->second;
							pid_quantum.erase(pid);
							pid_quantum.insert(pair<int, int> (pid, quantum[0]));
						}
		q.pop();
	} 
	
	return pid;
}