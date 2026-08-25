CXX      ?= g++
MPICXX   ?= mpicxx
CXXFLAGS ?= -std=c++14 -O3 -march=x86-64-v3 -Wall -Wextra
OMPFLAG  ?= -fopenmp

# Aggiungeremo dataset.cpp e shrink_omp.cpp man mano che li creiamo
SRC_COMMON = 
HDRS = src/common.hpp

all: hclust_seq hclust_mpi

# Target sequenziale per la verifica di correttezza (senza MPI)
hclust_seq: tests/test_sequential.cpp $(SRC_COMMON) $(HDRS)
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sequential.cpp $(SRC_COMMON)

# Target ibrido MPI + OpenMP
hclust_mpi: src/main.cpp $(SRC_COMMON) $(HDRS)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAG) -DUSE_MPI -o $@ src/main.cpp $(SRC_COMMON)

clean:
	rm -f hclust_seq hclust_mpi *.o

.PHONY: all clean