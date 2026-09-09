#ifndef COMMON_HPP
#define COMMON_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <omp.h>
#include <vector>
#include <atomic>

#ifdef USE_MPI
#include <mpi.h>    
#endif

//represents an edge or merge in the dendrogram
typedef struct {
    int u;       //first cluster or point id
    int v;       //second cluster or point id
    float dist;  //distance when merged
} Edge;

#endif //common_hpp