export LC_ALL=C
CXXFLAGS= -Wall -pedantic -g -ggdb
LDFLAGS= -lpthread

OBJS=main.o simu.o basesched.o basetask.o tasks.o sched_rr.o sched_fcfs.o sched_rr2.o sched_no_mistery.o
MAIN=simusched

.PHONY: all clean new
all: $(MAIN)

$(MAIN): $(OBJS) sched_mistery.o 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


%.o: %.cpp %.h basesched.h
basesched.o: basesched.cpp basesched.h
basetask.o: basetask.cpp basetask.h
main.o: main.cpp simu.h basetask.h basesched.h tasks.h sched_fcfs.h \
 sched_rr.h sched_rr2.h
simu.o: simu.cpp simu.h basetask.h basesched.h
simusched.o: simusched.cpp basetask.h basesched.h tasks.h
tasks.o: tasks.cpp tasks.h basetask.h

clean:
	rm -f $(OBJS) $(MAIN)

new: clean all
main_solo: main.cpp simu.h

ej= main_solo simu.o basesched.o basetask.o tasks.o sched_fcfs.o simusched

ej1: $(ej)
	mkdir Ejercicio1
	./simusched ./Lotes/Ej1 1 1 1 SchedFCFS | ./graphsched.py >> ./Ejercicio1/TaskConsola.png

cleanej1:
	rm -f $(OBJS) $(MAIN) ./Ejercicio1/TaskConsola.png
	rm -d -f ./Ejercicio1

ej2: $(ej)
	mkdir Ejercicio2
	./simusched ./Lotes/Ej2 1 4 1 SchedFCFS | ./graphsched.py >> ./Ejercicio2/fcfs1.png
	./simusched ./Lotes/Ej2 2 4 1 SchedFCFS | ./graphsched.py >> ./Ejercicio2/fcfs2.png


cleanej2:
	rm -f $(OBJS) $(MAIN) ./Ejercicio2/fcfs1.png
	rm -f ./Ejercicio2/fcfs2.png
	rm -d -f ./Ejercicio2

ej3: $(ej)
	mkdir Ejercicio3
	./simusched ./Lotes/Ej3 1 1 1 SchedFCFS | ./graphsched.py >> ./Ejercicio3/TaskBatch.png

cleanej3:
	rm -f $(OBJS) $(MAIN) ./Ejercicio3/TaskBatch.png
	rm -d -f ./Ejercicio3

ej4: $(ej)
	mkdir Ejercicio4
	./simusched ./Lotes/Ej4 1 1 1 SchedRR 5 | ./graphsched.py >> ./Ejercicio4/RR.png

cleanej4:
	rm -f $(OBJS) $(MAIN) ./Ejercicio4/RR.png

ej5: $(ej)
	mkdir Ejercicio5
	./simusched ./Lotes/Ej5 1 2 1 SchedRR 2 | ./graphsched.py >> ./Ejercicio5/RR_Q2.png
	./simusched ./Lotes/Ej5 1 2 1 SchedRR 10 | ./graphsched.py >> ./Ejercicio5/RR_Q10.png
	./simusched ./Lotes/Ej5 1 2 1 SchedRR 50 | ./graphsched.py >> ./Ejercicio5/RR_Q50.png

cleanej5:
	rm -f $(OBJS) $(MAIN) ./Ejercicio5/RR_Q2.png
	rm -f ./Ejercicio5/RR_Q10.png
	rm -f ./Ejercicio5/RR_Q50.png
	rm -d -f ./Ejercicio5

ej6: $(ej)
	mkdir Ejercicio6
	./simusched ./Lotes/Ej6 1 2 1 SchedFCFS | ./graphsched.py >> ./Ejercicio6/FCFS.png

cleanej6:
	rm -f $(OBJS) $(MAIN) ./Ejercicio6/FCFS.png
	rm -d -f ./Ejercicio6

ej7: $(ej)
	mkdir Ejercicio7
	./simusched ./Lotes/mistery1 1 1 1 SchedMistery 6 | ./graphsched.py >> ./Ejercicio7/mistery1.png
	./simusched ./Lotes/mistery3 1 1 1 SchedMistery 5 3 2 | ./graphsched.py >> ./Ejercicio7/mistery2.png

cleanej7:
	rm -f $(OBJS) $(MAIN) ./Ejercicio7/mistery1.png
	rm -f ./Ejercicio7/mistery2.png
	rm -d -f ./Ejercicio7

ej8: $(ej)
	mkdir Ejercicio8
	./simusched ./Lotes/ej8lote2 2 1 1 SchedRR 5 5 | ./graphsched.py >> ./Ejercicio8/SchedRR1_1.png
	./simusched ./Lotes/ej8lote2 2 1 1 SchedRR2 5 5 | ./graphsched.py >> ./Ejercicio8/SchedRR2_1.png
	./simusched ./Lotes/ej8lote3 2 1 5 SchedRR 3 3 | ./graphsched.py >> ./Ejercicio8/SchedRR1_2.png
	./simusched ./Lotes/ej8lote3 2 1 5 SchedRR2 3 3 | ./graphsched.py >> ./Ejercicio8/SchedRR2_2.png

cleanej8:
	rm -f $(OBJS) $(MAIN) ./Ejercicio8/SchedRR1_1.png
	rm -f ./Ejercicio8/SchedRR2_1.png
	rm -f ./Ejercicio8/SchedRR1_2.png
	rm -f ./Ejercicio8/SchedRR2_2.png
	rm -d -f ./Ejercicio8

cleanTodosLosEjercicios:
	rm -f $(OBJS) $(MAIN) ./Ejercicio1/TaskConsola.png
	rm -f ./Ejercicio2/fcfs1.png
	rm -f ./Ejercicio2/fcfs2.png
	rm -f ./Ejercicio3/TaskBatch.png
	rm -f ./Ejercicio4/RR.png
	rm -f ./Ejercicio5/RR_Q2.png
	rm -f ./Ejercicio5/RR_Q10.png
	rm -f ./Ejercicio5/RR_Q50.png
	rm -f ./Ejercicio6/FCFS.png
	rm -f ./Ejercicio7/mistery1.png
	rm -f ./Ejercicio7/mistery2.png
	rm -f ./Ejercicio8/SchedRR1_1.png
	rm -f ./Ejercicio8/SchedRR2_1.png
	rm -f ./Ejercicio8/SchedRR1_2.png
	rm -f ./Ejercicio8/SchedRR2_2.png

	rm -d -f ./Ejercicio1
	rm -d -f ./Ejercicio2
	rm -d -f ./Ejercicio3
	rm -d -f ./Ejercicio4
	rm -d -f ./Ejercicio5
	rm -d -f ./Ejercicio6
	rm -d -f ./Ejercicio7
	rm -d -f ./Ejercicio8
