# Scheduling project

## Build and run

There are two kinds of builds, release and debug. Debug builds print debug
information and run the executable with valgrind.

* Building individual executable:

    - make scheduler
    - make bischeduler

* Dynamic Programming Scheduler build and run:
    
    - make release-sched CLI=i6.dat
    - make debug-sched CLI=i6.dat

* FPTAS Scheduler build and run:
    
    - make release-sched CLI="i6.dat 1"
    - make debug-sched CLI="i50b.dat 525"

* Biobjective scheduler build and run:

    - make release-bisched CLI="i6.dat"
    - make debug-bisched CLI="i6.dat"
