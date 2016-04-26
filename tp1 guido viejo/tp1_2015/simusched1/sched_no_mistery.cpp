#include <vector>
#include <queue>
#include "sched_no_mistery.h"
#include "basesched.h"
#include <iostream>
#include <stdio.h>

using namespace std;


SchedNoMistery::SchedNoMistery(vector<int> argn) {

	// Mistery recibe la cantidad de quantum por parámetro, inicializamos los quantum
	contQuantumPasados = 0;
	for (int i = 0; i < argn.size() ; i++){
		quantum.push_back(argn[i]);

	}

}


SchedNoMistery::~SchedNoMistery() {
}


void SchedNoMistery::load(int pid) {

	//LLega una nueva tarea, dependiendo si llego tarde o no se le asigna el valor de quantum correspondiente
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
		//lo inserto ordenado acorde a la prioridad 
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
		q.push(pid);
		pid_Tarde.insert(pair<int, int> (pid, 1));
	}else{
		pid_quantum.insert(pair<int, int> (pid, quantum[0]));
		pid_Tarde.insert(pair<int, int> (pid, 0));
		q.push(pid); 
	}	
}

void SchedNoMistery::unblock(int pid) {
	//la tarea se desbloqueo, la pusheo a la cola para q vuelva a correr
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
/*		llega un tick de reloj, chequeamos que esta corriendo en el cpu,
		si es la idle vemos en nuestras colas si hay alguien listo para correr
*/
			if (current_pid(cpu) == IDLE_TASK) {
				if (!q.empty()) {
					sig = q.front(); q.pop();
					pidInicial = sig;
					quantumActual = (pid_quantum.find(sig))->second;
					return sig;
				} else {
					if (!desbloqueados.empty())	{
						quantumActual = 1;
						sig = desbloqueados.front();
						desbloqueados.pop();
						return sig;
					}
					return IDLE_TASK;
				}
			} else {

		/*Si no es la idle restamos quantum y chequeamos si se le termino el mismo
		en caso de terminar, chequeamos si la tarea que acaba de correr era una q habia 
		llegado tarde, si esto es correcto la reordenamos para q vuelva el orden ciclico correcto
		(1º 2º 3º 4º....) luego, chequeeamos si hay alguien desbloqueado para correr primero (asignandole
		el quantum correspondiente igual a 1) y luego
		si hay alguien que llego tarde (asignandole el quantum correspondiente igual a la suma
		de los quantum que ya corrieron) . Si no hay ninguna tarea en ninguna de estas dos situaciones
		mandamos a correr la que esta primera en la cola q
		A su vez, revisamos si la proxima tarea a correr es la primera de nuestro orden ciclico
		sumamos uno a nuestro contador para mover el vector de quantum
		*/		
				quantumActual--;


				if (quantumActual == 0) {
				
					if (pid_Tarde.find(current_pid(cpu))->second){
						pid_Tarde.erase(current_pid(cpu));
						pid_Tarde.insert(pair <int, int> (current_pid(cpu),0));
					}else{
						q.push(current_pid(cpu));
					}

						if (!desbloqueados.empty())	{
							quantumActual = 1;
							sig = desbloqueados.front();
							desbloqueados.pop();
							return sig;
						}
						if ((pid_quantum.find(q.front()))->second == quantum[0]) {
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = (pid_quantum.find(q.front()))->second;
							pid_quantum.erase(q.front());
							pid_quantum.insert(pair<int, int> (q.front(), quantum[0]));
						}

					if (q.front() == pidInicial) {
						if (contQuantumPasados < quantum.size()-1)	{
							contQuantumPasados++;
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = quantum.back();
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
	/*Aqui chequeamos si al finalizar la tarea o bloquearse si queda alguien para corerr 
	o si va la idle, y ademas chequeamos si la tarea q se bloqueo o finalizo es la primera 
	de nuestro orden ciclico, si es, guardamos en nuestro entero pidinicial el pid de la 
	siguiente tarea*/
	int pid ;
	if (q.empty() && desbloqueados.empty()){ 
		pid= IDLE_TASK;  //no hay mas tareas -.-> idle_task
	}else {
		if (current_pid(cpu) == pidInicial)	{
			pidInicial = q.front();
		}
		if (!desbloqueados.empty())	{
							quantumActual = 1;
							pid = desbloqueados.front();
							desbloqueados.pop();
							return pid;
						}
		
		pid = q.front();	//saco la tarea de la cola
		
		if ((pid_quantum.find(pid))->second == quantum[0]) {
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = (pid_quantum.find(pid))->second;
							pid_quantum.erase(pid);
							pid_quantum.insert(pair<int, int> (pid, quantum[0]));
						}
		if (pid == pidInicial) {
						if (contQuantumPasados < quantum.size()-1)	{
							contQuantumPasados++;
							quantumActual = quantum[contQuantumPasados];
						}else{
							quantumActual = quantum.back();
						}
					}
		q.pop();
	} 
	return pid;
}