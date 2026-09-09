#include "common.hpp"
#include <mpi.h>

//creates and returns the MPI derived type for the Edge struct
MPI_Datatype create_mpi_edge_type() {
    MPI_Datatype mpi_edge_type;
    
    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_FLOAT};
    MPI_Aint displacements[3];

    displacements[0] = offsetof(Edge, u);
    displacements[1] = offsetof(Edge, v);
    displacements[2] = offsetof(Edge, dist);

    MPI_Type_create_struct(3, blocklengths, displacements, types, &mpi_edge_type);
    MPI_Type_commit(&mpi_edge_type);

    return mpi_edge_type;
}

int main(int argc, char** argv) {
    int provided;
    
    //multithread support request
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

    if (provided < MPI_THREAD_FUNNELED) {
        fprintf(stderr, "Errore: l'implementazione MPI non supporta MPI_THREAD_FUNNELED.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Datatype MPI_EDGE = create_mpi_edge_type();

    /* 
     * ==========================================
     * mock data generation (1d block partitioning)
     * ==========================================
     */
    const int N_TOTAL = 10000;//total number of rows (points) to simulate
    const int D = 18;//number of dimensions (e.g., susy has 18 features)

    //calculation of the local block for the current process
    int local_N = N_TOTAL / size;
    int remainder = N_TOTAL % size;
    
    //distribution of the remainder to the first 'remainder' ranks
    if (rank < remainder) {
        local_N += 1;
    }

    //contiguous allocation for the local block (1d flat array structure for cache efficiency)
    float* local_data = (float*)malloc(local_N * D * sizeof(float));
    if (!local_data) {
        fprintf(stderr, "Errore: Memoria esaurita sul rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    //populating with random data (seed based on rank to diversify data)
    srand(42 + rank);
    for (int i = 0; i < local_N * D; i++) {
        local_data[i] = (float)rand() / RAND_MAX;
    }

    printf("Rank %d di %d: allocati %d punti (%.2f MB).\n", 
           rank, size, local_N, (local_N * D * sizeof(float)) / (1024.0 * 1024.0));

    /* 
     * ==========================================
     * todo: openmp phase (shrink)
     * ==========================================
     */

    free(local_data);

    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}