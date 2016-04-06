#include <vector>
#include <queue>
#include "sched_no_mistery.h"
#include "basesched.h"
#include <iostream>
#include <stdio.h>

using namespace std;


SchedNoMistery::SchedNoMistery(vector<int> argn) {

	// Mistery recibe la cantidad de quantum por parámetro
	//contQuantumPasados = 0;
	quantumActual = 0;
	for (int i = 0; i < argn.size() ; i++){
		std::queue<int> QuantumAux;
		quantumDeTareas.push_back(QuantumAux); // Agrego una queue para cada quantum mas una inicial
	}
	for (int i = 0; i < argn.size(); ++i){
		quantum.push_back(argn[i]); // Agrego la lista de todos los quantums
		
	}
	
	
}


SchedNoMistery::~SchedNoMistery() {
}


void SchedNoMistery::load(int pid) {
	if(quantumActual != 0){
		tareaEntroTarde.push(pid);
	}
	for (int i = quantumActual; i < quantumDeTareas.size(); ++i){
		quantumDeTareas[i].push(pid);
		//q.push(pid);

	}
}

void SchedNoMistery::unblock(int pid) {
	bloqueados.push(pid);
}

int SchedNoMistery::tick(int cpu, const enum Motivo m) {
	int sig;

	switch (m) {
		case EXIT:
			return next(cpu);
			break;
		case BLOCK:
			quantumDeTareas[quantumActual].pop();
			return next(cpu);
			break;
		case TICK:
			if (current_pid(cpu) == IDLE_TASK){
				if (sinTareas() && tareaEntroTarde.empty() && bloqueados.empty()){ 
					sig = IDLE_TASK;  //no hay mas tareas -.-> idle_task
					return sig;
				}else{
					if(!tareaEntroTarde.empty()){
						sig = tareaEntroTarde.front();
						tareaEntroTarde.pop();
						quantumTarea = 0;
						for (int i = 0; i < quantumActual - 1; ++i)	{
							quantumTarea += quantum[i];
						}
						quantumRestante = quantumTarea;
						return sig;
					}else{
						if (!bloqueados.empty()){
							sig = bloqueados.front();
							bloqueados.pop();
							quantumRestante = 1;
							if(quantumActual >= quantum.size()-1){
								quantumDeTareas[quantumActual].push(current_pid(cpu));
							}
							return sig;
						}else{
							if(quantumDeTareas[quantumActual].empty()){
								quantumActual++;
								sig = quantumDeTareas[quantumActual].front();
								quantumDeTareas[quantumActual].pop();
								//q.pop();
								if(quantumActual >= quantum.size()-1){
								quantumDeTareas[quantumActual].push(sig);
								//q.push(sig);
								}
								quantumTarea = quantum[quantumActual];
								quantumRestante = quantumTarea;
								return sig;
							}else{
								sig = quantumDeTareas[quantumActual].front();
								quantumDeTareas[quantumActual].pop();
								//q.pop();
								if(quantumActual >= quantum.size()-1){
								quantumDeTareas[quantumActual].push(sig);
								//q.push(sig);
								}
								quantumTarea = quantum[quantumActual];
								quantumRestante = quantumTarea;
								return sig;
							}

						}
					}	
				} 
			} else {
				quantumRestante--;
				if (quantumRestante == 0) {
					if(!tareaEntroTarde.empty()){
					sig = tareaEntroTarde.front();
					tareaEntroTarde.pop();
					if(quantumActual >= quantum.size()-1){
						quantumDeTareas[quantumActual].push(current_pid(cpu));
					}
					quantumTarea = 0;
					for (int i = 0; i < quantumActual; ++i)	{
						quantumTarea += quantum[i];
					}
					quantumRestante = quantumTarea;
					return sig;
				}else{
					if (!bloqueados.empty()) {
						sig = bloqueados.front();
						bloqueados.pop();
						quantumRestante = 1;
						if(quantumActual >= quantum.size()-1){
						quantumDeTareas[quantumActual].push(current_pid(cpu));
						}
						return sig;
					}else{
						if(quantumActual >= quantum.size()-1){	
							
								quantumDeTareas[quantumActual].push(current_pid(cpu));
								quantumTarea = quantum[quantumActual];
							}

							if(quantumDeTareas[quantumActual].empty()){

								if(quantumActual != (quantum.size() -1) ){
								quantumActual++;
								}
							}
							sig = quantumDeTareas[quantumActual].front();
							
							quantumDeTareas[quantumActual].pop();
							//if(quantumActual >= quantum.size()-1){
							//	quantumDeTareas[quantumActual].push(current_pid(cpu));
							//}
							quantumTarea = quantum[quantumActual];
							quantumRestante = quantumTarea;

							
							return sig;
					}
					
				}		
				} else {
					return current_pid(cpu);
				}
			}
			break;
	}
}

int SchedNoMistery::next(int cpu){
	int sig;

	if (!bloqueados.empty()){
		sig = bloqueados.front();
		bloqueados.pop();
		quantumRestante = 1;

		return sig;
	}else{

		if(!tareaEntroTarde.empty()){
			sig = tareaEntroTarde.front();
			tareaEntroTarde.pop();
			quantumTarea = 0;
			for (int i = 0; i < quantumActual - 1; ++i)	{
				quantumTarea += quantum[i];
			}
			quantumRestante = quantumTarea;
			return sig;
			}else{
				if(sinTareas()){
					return IDLE_TASK;
				}else{
					if(quantumDeTareas[quantumActual].empty()){
						quantumActual++;
					}
						sig = quantumDeTareas[quantumActual].front();
						quantumDeTareas[quantumActual].pop();
						//q.pop();
						quantumTarea = quantum[quantumActual];
						quantumRestante = quantumTarea;
						return sig;
					}
			}	
	}
	return sig;
}

bool SchedNoMistery::sinTareas(){
	for (int i = 0; i < quantum.size(); ++i) {
		if(!quantumDeTareas[i].empty()){
			return false;
		}
	}
	return true;
}