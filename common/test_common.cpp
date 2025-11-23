#include "CommonGraph.h"
#include "CommonAlgorithms.h"
#include <iostream>
#include <cassert>

void test_bfs() {
    std::cout << "Testing BFS and Graph..." << std::endl;
    // Manual graph creation
    Graph g;
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(0, 2); // Shortcut
    g.add_edge(2, 3);

    SSSPResult res = BFS(g, 0);
    
    // 0 -> 1 is dist 1
    assert(res.distances[1] == 1);
    // 0 -> 2 is dist 1 (via direct edge)
    assert(res.distances[2] == 1);
    // 0 -> 3 is dist 2 (0->2->3)
    assert(res.distances[3] == 2);
    
    // Test Path Reconstruction
    Path p = reconstruct_path(res, 0, 3);
    assert(p.size() == 3); // 0, 2, 3
    assert(p[0] == 0);
    assert(p[2] == 3);
    
    std::cout << "BFS Passed!" << std::endl;
}

void test_dijkstra() {
    std::cout << "Testing Dijkstra..." << std::endl;
    WeightedAdjacencyList wg;
    // 0 -> 1 (weight 10)
    wg[0].push_back({1, 10});
    // 0 -> 2 (weight 2)
    wg[0].push_back({2, 2});
    // 2 -> 1 (weight 3) -> Total 0->2->1 = 5, which is < 10
    wg[2].push_back({1, 3});

    SSSPResult res = Dijkstra(wg, 0);

    assert(res.distances[1] == 5); // Should take the shortcut via 2
    assert(res.distances[2] == 2);

    std::cout << "Dijkstra Passed!" << std::endl;
}

void test_graph_modifications() {
    std::cout << "Testing Graph Modifications..." << std::endl;
    Graph g;
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);
    
    Path p = {0, 1, 2, 3};
    
    // Test Remove Path
    Graph g_minus = g.create_graph_minus_path(p);
    // Edge 0->1 should be gone
    // BFS from 0 should not reach 1
    SSSPResult res = BFS(g_minus, 0);
    assert(res.distances[1] == INF);
    
    // Test Reverse
    Graph g_rev = g.create_reversed_graph();
    SSSPResult res_rev = BFS(g_rev, 3);
    // 3->2->1->0 should exist now
    assert(res_rev.distances[0] == 3);

    std::cout << "Graph Mods Passed!" << std::endl;
}

int main() {
    test_bfs();
    test_dijkstra();
    test_graph_modifications();
    std::cout << "ALL TESTS PASSED." << std::endl;
    return 0;
}
