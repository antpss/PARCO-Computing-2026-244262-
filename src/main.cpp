#include "common.hpp"

//create and restituisce il tipo derivato MPI per la struct Edge
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
     * TODO: Generazione mock data, OpenMP SHRINK, MPI Binomial Tree
     */

    MPI_Type_free(&MPI_EDGE);
    MPI_Finalize();
    
    return EXIT_SUCCESS;
}