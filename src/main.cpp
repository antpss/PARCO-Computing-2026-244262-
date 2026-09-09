#include "common.hpp"
#include "shrink_omp.hpp"
#include <mpi.h>
//register custom mpi struct for edge
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
    
    //ask for multithread support
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

    if (provided < MPI_THREAD_FUNNELED) {
        fprintf(stderr, "Error: MPI implementation does not support MPI_THREAD_FUNNELED.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Datatype MPI_EDGE = create_mpi_edge_type();

    //generate dummy data with 1d block partition
    const int N_TOTAL = 10000;//total points to simulate
    const int D = 18;//susy dimensions

    //local slice size
    int local_N = N_TOTAL / size;
    int remainder = N_TOTAL % size;
    
    //give extra points to first ranks
    if (rank < remainder) {
        local_N += 1;
    }

    //flat buffer for local block
    float* local_data = (float*)malloc(local_N * D * sizeof(float));
    if (!local_data) {
        fprintf(stderr, "Error: Out of memory on rank %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    //fill with dummy random data
    srand(42 + rank);
    for (int i = 0; i < local_N * D; i++) {
        local_data[i] = (float)rand() / RAND_MAX;
    }

    printf("Rank %d of %d: allocated %d points (%.2f MB).\n", 
           rank, size, local_N, (local_N * D * sizeof(float)) / (1024.0 * 1024.0));

    //run openmp shrink phase
    int K = 4;//split into 4 subsets for now
    std::vector<Edge> local_dendrogram = shrink_omp_process(local_data, local_N, D, K);

    printf("Rank %d of %d: OpenMP phase completed, generated %zu edges.\n", 
           rank, size, local_dendrogram.size());

    free(local_data);

    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}