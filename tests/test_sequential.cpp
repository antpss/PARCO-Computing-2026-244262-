#include "../src/common.hpp"
#include "../src/shrink_omp.hpp"
#include <stdio.h>
#include <vector>

int main() {
    printf("Esecuzione test sequenziale (Placeholder).\n");
    
    //simple 4-point 2d toy dataset
    float data[] = {
        0.0f, 0.0f,   //point 0
        0.0f, 1.0f,   //point 1 (dist to 0 is 1.0)
        10.0f, 10.0f, //point 2 (far away)
        10.0f, 11.0f  //point 3 (dist to 2 is 1.0)
    };
    
    int N = 4;
    int D = 2;
    int K = 2;
    
    std::vector<Edge> mst = shrink_omp_process(data, N, D, K);
    
    printf("Generated MST with %zu edges:\n", mst.size());
    for (const auto& e : mst) {
        printf("Edge: %d - %d (dist^2: %.2f)\n", e.u, e.v, e.dist);
    }
    
    return 0;
}