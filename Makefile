CXX      ?= g++
MPICXX   ?= mpicxx
CXXFLAGS ?= -std=c++14 -O3 -march=x86-64-v3 -Wall -Wextra
OMPFLAG  ?= -fopenmp

# We will add dataset.cpp and shrink_omp.cpp as we create them
SRC_COMMON = 
HDRS = src/common.hpp

all: hclust_seq hclust_mpi

# Sequential target for correctness verification (without MPI)
hclust_seq: tests/test_sequential.cpp $(SRC_COMMON) $(HDRS)
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sequential.cpp $(SRC_COMMON)

# Hybrid MPI + OpenMP target
hclust_mpi: src/main.cpp $(SRC_COMMON) $(HDRS)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAG) -DUSE_MPI -o $@ src/main.cpp $(SRC_COMMON)

clean:
	rm -f hclust_seq hclust_mpi *.o

.PHONY: all clean