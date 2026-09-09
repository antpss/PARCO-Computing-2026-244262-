#include "shrink_omp.hpp"
#include <cmath>
#include <algorithm>

//squared euclidean distance between two points
static float calc_dist_sq(const float* data, int u, int v, int D) {
    float dist_sq = 0.0f;
    for (int d = 0; d < D; ++d) {
        float diff = data[u * D + d] - data[v * D + d];
        dist_sq += diff * diff;
    }
    return dist_sq;
}

//prim algorithm to compute mst on one or two subsets
static void compute_partial_mst(
    const float* data, int D,
    int subset_i_start, int subset_i_size,
    int subset_j_start, int subset_j_size,
    std::vector<Edge>& local_edges) {

    int total_nodes = subset_i_size + subset_j_size;
    if (total_nodes <= 1) return;

    //map local index in prim back to global point index
    std::vector<int> global_idx(total_nodes);
    for (int i = 0; i < subset_i_size; ++i) {
        global_idx[i] = subset_i_start + i;
    }
    for (int j = 0; j < subset_j_size; ++j) {
        global_idx[subset_i_size + j] = subset_j_start + j;
    }

    std::vector<float> min_dist(total_nodes, INFINITY);
    std::vector<int> parent(total_nodes, -1);
    std::vector<bool> in_mst(total_nodes, false);

    //start from node 0
    min_dist[0] = 0.0f;

    for (int count = 0; count < total_nodes; ++count) {
        //find closest unvisited node
        float min = INFINITY;
        int u = -1;

        for (int v = 0; v < total_nodes; v++) {
            if (!in_mst[v] && min_dist[v] < min) {
                min = min_dist[v];
                u = v;
            }
        }

        if (u == -1) break; //disconnected, shouldn't really happen
        in_mst[u] = true;

        if (parent[u] != -1) {
            Edge e;
            e.u = global_idx[parent[u]];
            e.v = global_idx[u];
            e.dist = min_dist[u]; 
            local_edges.push_back(e);
        }

        //relax neighbors with the newly picked node
        for (int v = 0; v < total_nodes; v++) {
            if (!in_mst[v]) {
                float dist = calc_dist_sq(data, global_idx[u], global_idx[v], D);
                if (dist < min_dist[v]) {
                    parent[v] = u;
                    min_dist[v] = dist;
                }
            }
        }
    }
}

std::vector<Edge> shrink_omp_process(float* data, int N, int D, int K) {
    if (N <= 1) return {};
    
    //fallback if k is weird or n is small
    if (K > N || K <= 0) K = std::max(1, N / 100);

    int subset_size = (N + K - 1) / K; //round up chunk size
    
    std::vector<Edge> all_edges;
    
    //collect edges locally per thread to avoid lock contention
    
    #pragma omp parallel
    {
        std::vector<Edge> thread_local_edges;

        #pragma omp for schedule(dynamic)
        for (int pair_idx = 0; pair_idx < (K * (K + 1)) / 2; ++pair_idx) {
            //map 1d loop counter to subset pair (i, j)
            int i = 0;
            int j = 0;
            int current_idx = 0;
            bool found = false;
            for (int r = 0; r < K; ++r) {
                for (int c = r; c < K; ++c) {
                    if (current_idx == pair_idx) {
                        i = r;
                        j = c;
                        found = true;
                        break;
                    }
                    current_idx++;
                }
                if (found) break;
            }
            
            int subset_i_start = i * subset_size;
            int subset_i_size = std::min(subset_size, N - subset_i_start);
            
            int subset_j_start = j * subset_size;
            int subset_j_size = std::min(subset_size, N - subset_j_start);

            if (subset_i_size > 0 && subset_j_size > 0) {
                //if i == j mst on single subset, else on the union of i and j
                if (i == j) {
                    compute_partial_mst(data, D, subset_i_start, subset_i_size, 0, 0, thread_local_edges);
                } else {
                    compute_partial_mst(data, D, subset_i_start, subset_i_size, subset_j_start, subset_j_size, thread_local_edges);
                }
            }
        }

        #pragma omp critical
        {
            all_edges.insert(all_edges.end(), thread_local_edges.begin(), thread_local_edges.end());
        }
    }

    //sort all candidate edges by distance
    std::sort(all_edges.begin(), all_edges.end(), [](const Edge& a, const Edge& b) {
        return a.dist < b.dist;
    });

    std::vector<Edge> final_mst;
    final_mst.reserve(N - 1);
    
    ConcurrentUnionFind uf(N);

    //kruskal pass over sorted edges to build the final mst
    for (const auto& e : all_edges) {
        if (uf.unite(e.u, e.v)) {
            final_mst.push_back(e);
            if (final_mst.size() == (size_t)(N - 1)) break;
        }
    }

    return final_mst;
}
