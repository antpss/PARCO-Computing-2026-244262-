CXX      ?= g++
MPICXX   ?= mpicxx
CXXFLAGS ?= -std=c++14 -O3 -march=x86-64-v3 -Wall -Wextra
OMPFLAG  ?= -fopenmp

#we'll add dataset.cpp and shrink_omp.cpp as we go
SRC_COMMON = src/shrink_omp.cpp
SRC_MPI = src/mpi_reduction.cpp
HDRS = src/common.hpp

all: hclust_seq hclust_mpi

#sequential test without mpi
hclust_seq: tests/test_sequential.cpp $(SRC_COMMON) $(HDRS)
	$(CXX) $(CXXFLAGS) $(OMPFLAG) -o $@ tests/test_sequential.cpp $(SRC_COMMON)

#hybrid mpi + openmp build
hclust_mpi: src/main.cpp $(SRC_COMMON) $(SRC_MPI) $(HDRS)
	$(MPICXX) $(CXXFLAGS) $(OMPFLAG) -DUSE_MPI -o $@ src/main.cpp $(SRC_COMMON) $(SRC_MPI)

clean:
	rm -f hclust_seq hclust_mpi *.o

.PHONY: all clean