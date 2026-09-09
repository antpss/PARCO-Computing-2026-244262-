#ifndef SHRINK_OMP_HPP
#define SHRINK_OMP_HPP

#include "common.hpp"

//thread-safe union-find with atomics for lock-free ops
class ConcurrentUnionFind {
private:
    std::vector<std::atomic<int>> parent;

public:
    ConcurrentUnionFind(int n) : parent(n) {
        for (int i = 0; i < n; ++i) {
            parent[i].store(i, std::memory_order_relaxed);
        }
    }

    //find root of i with path compression
    int find(int i) {
        int p = parent[i].load(std::memory_order_relaxed);
        if (p == i) return i;
        
        int root = find(p);
        
        //compress path with cas
        int curr = i;
        while (curr != root) {
            int nxt = parent[curr].load(std::memory_order_relaxed);
            //if cas fails someone else updated it, all good
            parent[curr].compare_exchange_weak(nxt, root, std::memory_order_relaxed);
            curr = nxt;
        }
        return root;
    }

    //merge sets of i and j, returns false if already in same set
    bool unite(int i, int j) {
        while (true) {
            int root_i = find(i);
            int root_j = find(j);
            
            if (root_i == root_j) return false;
            
            //point higher root to lower to avoid cycles
            if (root_i > root_j) {
                std::swap(root_i, root_j);
            }
            
            int expected = root_j;
            if (parent[root_j].compare_exchange_strong(expected, root_i, std::memory_order_relaxed)) {
                return true;
            }
        }
    }
};

//local shrink step with openmp across subsets
std::vector<Edge> shrink_omp_process(float* data, int N, int D, int K);

#endif //shrink_omp_hpp
