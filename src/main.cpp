#include "common.hpp"
#include "shrink_omp.hpp"
#include "mpi_reduction.hpp"
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

    int global_N = 0;
    MPI_Allreduce(&local_N, &global_N, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    int global_start_index = 0;
    MPI_Exscan(&local_N, &global_start_index, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    if (rank == 0) {
        global_start_index = 0;
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

    MPI_Barrier(MPI_COMM_WORLD);
    double t_omp_start = MPI_Wtime();

    //run openmp shrink phase
    int K = 4;//split into 4 subsets for now
    std::vector<Edge> local_dendrogram = shrink_omp_process(local_data, local_N, D, K);

    //offset local edge indices to global point indices
    for (auto& e : local_dendrogram) {
        e.u += global_start_index;
        e.v += global_start_index;
    }

    double t_omp_end = MPI_Wtime();
    double t_omp_local = t_omp_end - t_omp_start;
    double t_omp_max = 0.0;
    MPI_Reduce(&t_omp_local, &t_omp_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("OpenMP phase completed in %.4f seconds.\n", t_omp_max);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_mpi_start = MPI_Wtime();

    std::vector<Edge> global_dendrogram = mpi_binomial_tree_reduce(
        local_dendrogram, rank, size, global_N, MPI_EDGE
    );

    double t_mpi_end = MPI_Wtime();
    double t_mpi_local = t_mpi_end - t_mpi_start;
    double t_mpi_max = 0.0;
    MPI_Reduce(&t_mpi_local, &t_mpi_max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("MPI Reduction phase completed in %.4f seconds.\n", t_mpi_max);
        printf("Global dendrogram contains %zu edges (expected %d).\n", 
               global_dendrogram.size(), global_N - 1);
    }

    free(local_data);

    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}