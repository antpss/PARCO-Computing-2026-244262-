#ifndef MPI_REDUCTION_HPP
#define MPI_REDUCTION_HPP

#include "common.hpp"
#include <vector>
#include <mpi.h>

//executes a binomial tree reduction to merge local mst edges into a global mst on rank 0
std::vector<Edge> mpi_binomial_tree_reduce(
    std::vector<Edge>& local_edges, 
    int rank, 
    int size, 
    int N_total, 
    MPI_Datatype MPI_EDGE
);

#endif //mpi_reduction_hpp
