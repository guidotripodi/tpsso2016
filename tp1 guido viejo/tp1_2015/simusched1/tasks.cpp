#include "tasks.h"
#include <stdlib.h>
#include <iostream>
#include <time.h>
using namespace std;

void TaskCPU(int pid, vector<int> params) { // params: n
	uso_CPU(pid, params[0]); // Uso el CPU n milisegundos.
}

void TaskIO(int pid, vector<int> params) { // params: ms_pid, ms_io,
	uso_CPU(pid, params[0]); // Uso el CPU ms_pid milisegundos.
	uso_IO(pid, params[1]); // Uso IO ms_io milisegundos.
}

void TaskAlterno(int pid, vector<int> params) { // params: ms_pid, ms_io, ms_pid, ...
	for(int i = 0; i < (int)params.size(); i++) {
		if (i % 2 == 0) uso_CPU(pid, params[i]);
		else uso_IO(pid, params[i]);
	}
}

void TaskConBloqueo(int pid, vector<int> params) {
	int tiempo = params[0];
	int inicioBloq = params[1];
	int finBloq = params[2];

	if(inicioBloq > tiempo){
		uso_CPU(pid, tiempo);
	}else{
		uso_CPU(pid,inicioBloq-2);
		uso_IO(pid,finBloq-inicioBloq+1);
		uso_CPU(pid,tiempo-finBloq);
	}
}

void TaskConsola(int pid, vector<int> params) {
	int i, ciclos;

	for (i = 0; i < params[0]; i++) {
		ciclos = rand() % (params[2] - params[1] + 1) + params[1];
		uso_IO(pid, ciclos);
	}
}

void TaskBatch(int pid, vector<int> params) {
	int total_cpu = params[0];
	int cant_bloqueos = params[1];
	
	vector<bool> acciones = vector<bool>(total_cpu);
	 // Me creo un vector que va a representar los momentos en los que se use el uso_CPU o uso_IO.
	
	for(int i=0;i<(int)acciones.size();i++) 
		//segun internet poniendo .size me da el tamaño q tiene mi vector aunq en este caso podria usar total_cpu
		acciones[i] = false;
		
	for(int i=0;i<cant_bloqueos;i++) {
		int j = rand()%(acciones.size());
		if(!acciones[j])
			acciones[j] = true;
		else
			i--; // si no va a usar io, vuelvo el contador uno para atrás.
	}

	for(int i=0;i<(int)acciones.size();i++) {
		if( acciones[i] )
			uso_IO(pid,1); 
		else
			uso_CPU(pid, 1); // Uso el CPU durante 1 ciclo de reloj. (este no se si es necesario)
	}
}


void tasks_init(void) {
	/* Todos los tipos de tareas se deben registrar acá para poder ser usadas.
	 * El segundo parámetro indica la cantidad de parámetros que recibe la tarea
	 * como un vector de enteros, o -1 para una cantidad de parámetros variable. */
	register_task(TaskCPU, 1);
	register_task(TaskIO, 2);
	register_task(TaskAlterno, -1);
	register_task(TaskConBloqueo,3);
	register_task(TaskConsola, 3);
	register_task(TaskBatch, 2);
}
