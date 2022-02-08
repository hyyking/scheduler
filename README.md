# Scheduling project

## Build and run

There are two kinds of builds, release and debug. Debug builds print debug
informations and are run with valgrind.


* Dynamic Programming Scheduler:
    
    - make release-sched CLI=i6.dat
    - make debug-sched CLI=i6.dat

* FPTAS Scheduler:
    
    - release-fptas CLI="i6.dat 1"
    - debug-fptas CLI="i6.dat 1"


* Biobjective scheduler:

    - release-bisched CLI="i6.dat"
    - debug-bisched CLI="i6.dat"
