#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <omp.h>

#ifdef USE_MPI
#include <mpi.h>    
#endif

//struct which represents an edge/merge in the dendrogram
typedef struct {
    int u;       //ID of the first cluster/point
    int v;       //ID of the second cluster/point
    float dist;  //distance of single linkage at which the merge occurs
} Edge;

#endif //COMMON_HPP