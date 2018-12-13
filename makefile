EXECS_SEQ=kmeans_seq
EXECS_OPENMP=kmeans_openmp
EXECS_MPI=kmeans_mpi
EXECS_MPI_OPENMP=kmeans_mpi_openmp
GPP=g++
MPICPP=mpic++
COMMON=common.h common.cpp
FLAGS=-O3

all: ${EXECS_SEQ} ${EXECS_OPENMP} ${EXECS_MPI} ${EXECS_MPI_OPENMP}

kmeans_seq: kmeans_seq.cpp ${COMMON}
	${GPP} ${FLAGS} -o kmeans_seq kmeans_seq.cpp ${COMMON}

kmeans_openmp: kmeans_openmp.cpp ${COMMON}
	${GPP} ${FLAGS} -fopenmp -o kmeans_openmp kmeans_openmp.cpp ${COMMON}

kmeans_mpi: kmeans_mpi.cpp ${COMMON}
	${MPICPP} ${FLAGS} -o kmeans_mpi kmeans_mpi.cpp ${COMMON}

kmeans_mpi_openmp: kmeans_mpi_openmp.cpp ${COMMON}
	${MPICPP} ${FLAGS} -fopenmp -o kmeans_mpi_openmp kmeans_mpi_openmp.cpp ${COMMON}

clean:
	rm -f ${EXECS_SEQ} ${EXECS_OPENMP} ${EXECS_MPI} ${EXECS_MPI_OPENMP}

N_MPI_PROCS=4
N_POINTS=100000
N_CLUSTERS=10
N_DIMENSIONS=10
RUN_ARGS=${N_POINTS} ${N_CLUSTERS} ${N_DIMENSIONS}

run_seq: ${EXECS_SEQ}
	./${EXECS_SEQ} ${RUN_ARGS}

run_openmp: ${EXECS_OPENMP}
	./${EXECS_OPENMP} ${RUN_ARGS}

run_mpi: ${EXECS_MPI}
	mpirun -n ${N_MPI_PROCS} ./${EXECS_MPI} ${RUN_ARGS}

run_mpi_openmp: ${EXECS_MPI_OPENMP}
	mpirun -n ${N_MPI_PROCS} ./${EXECS_MPI_OPENMP} ${RUN_ARGS}

run_all: run_seq run_openmp run_mpi run_mpi_openmp