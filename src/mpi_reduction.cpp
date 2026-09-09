#include "mpi_reduction.hpp"
#include "shrink_omp.hpp"
#include <algorithm>

//merges two sets of edges and applies kruskal's using concurrent union-find
static std::vector<Edge> merge_dendrograms(
    const std::vector<Edge>& edges1, 
    const std::vector<Edge>& edges2, 
    int N_total) 
{
    std::vector<Edge> all_edges;
    all_edges.reserve(edges1.size() + edges2.size());
    
    //concatenate both arrays
    all_edges.insert(all_edges.end(), edges1.begin(), edges1.end());
    all_edges.insert(all_edges.end(), edges2.begin(), edges2.end());
    
    //sort combined edges by distance
    std::sort(all_edges.begin(), all_edges.end(), [](const Edge& a, const Edge& b) {
        return a.dist < b.dist;
    });

    std::vector<Edge> final_mst;
    final_mst.reserve(N_total - 1);
    
    //use existing union-find sequentially to extract mst
    ConcurrentUnionFind uf(N_total);

    for (const auto& e : all_edges) {
        if (uf.unite(e.u, e.v)) {
            final_mst.push_back(e);
            if (final_mst.size() == (size_t)(N_total - 1)) break;
        }
    }

    return final_mst;
}

std::vector<Edge> mpi_binomial_tree_reduce(
    std::vector<Edge>& local_edges, 
    int rank, 
    int size, 
    int N_total, 
    MPI_Datatype MPI_EDGE) 
{
    std::vector<Edge> current_edges = local_edges;

    for (int step = 1; step < size; step *= 2) {
        if ((rank % (2 * step)) == 0) {
            int recv_rank = rank + step;
            if (recv_rank < size) {
                int incoming_count = 0;
                MPI_Recv(&incoming_count, 1, MPI_INT, recv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                std::vector<Edge> incoming_edges(incoming_count);
                if (incoming_count > 0) {
                    MPI_Recv(incoming_edges.data(), incoming_count, MPI_EDGE, recv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                
                //merge received edges with our current mst
                current_edges = merge_dendrograms(current_edges, incoming_edges, N_total);
            }
        } else {
            int dest_rank = rank - step;
            int count = (int)current_edges.size();
            MPI_Send(&count, 1, MPI_INT, dest_rank, 0, MPI_COMM_WORLD);
            
            if (count > 0) {
                MPI_Send(current_edges.data(), count, MPI_EDGE, dest_rank, 0, MPI_COMM_WORLD);
            }
            break; //this rank is done with the reduction phase
        }
    }

    return current_edges;
}
