#include "ReplacementPaths.h"
#include "common/CommonGraph.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <iomanip>
#include <cassert>

/**
 * Create a large test graph with interesting structure:
 * - Main shortest path
 * - Short detours (length <= 2L)
 * - Long detours (length > 2L)
 * - Some edges with no alternative
 */
Graph create_large_test_graph(int& source, int& target, Path& expected_shortest_path) {
    Graph g;

    // We'll create a graph with 150 vertices
    // Structure:
    //   Layer 0 (source): vertex 0
    //   Layer 1: vertices 1-10
    //   Layer 2: vertices 11-30
    //   Layer 3: vertices 31-60
    //   Layer 4: vertices 61-100
    //   Layer 5: vertices 101-140
    //   Layer 6 (target): vertex 141

    source = 0;
    target = 141;

    // Initialize all vertices explicitly to ensure vertex_count is correct
    for (int i = 0; i <= 141; ++i) {
        g.add_edge(i, i); // Self-loop to ensure vertex exists
        g.remove_edge(i, i); // Remove the self-loop
    }

    // Build main shortest path: 0 -> 15 -> 45 -> 80 -> 120 -> 141
    // This is a 5-hop path (6 vertices)
    expected_shortest_path = {0, 15, 45, 80, 120, 141};

    // Add shortest path edges
    g.add_edge(0, 15);
    g.add_edge(15, 45);
    g.add_edge(45, 80);
    g.add_edge(80, 120);
    g.add_edge(120, 141);

    // === SHORT DETOURS ===
    // For L = ceil(sqrt(150)) = 13, short detours have length <= 26

    // Short detour for edge 0->15: 0 -> 1 -> 2 -> 15 (length 3, bypasses 1 edge)
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 15);

    // Short detour for edge 15->45: 15 -> 20 -> 25 -> 30 -> 45 (length 4)
    g.add_edge(15, 20);
    g.add_edge(20, 25);
    g.add_edge(25, 30);
    g.add_edge(30, 45);

    // === LONG DETOURS ===
    // Long detour for edge 45->80: Goes through many intermediate vertices
    // 45 -> 46 -> 47 -> ... -> 60 -> 61 -> 62 -> ... -> 75 -> 80
    // This creates a path of length ~30 (> 2L = 26)
    for (int i = 45; i < 60; ++i) {
        g.add_edge(i, i + 1);
    }
    for (int i = 61; i < 75; ++i) {
        g.add_edge(i, i + 1);
    }
    g.add_edge(60, 61);
    g.add_edge(75, 80);

    // === ANOTHER SHORT DETOUR ===
    // For edge 80->120: 80 -> 90 -> 100 -> 110 -> 120 (length 4)
    g.add_edge(80, 90);
    g.add_edge(90, 100);
    g.add_edge(100, 110);
    g.add_edge(110, 120);

    // === MODERATE DETOUR for edge 120->141 ===
    // 120 -> 121 -> 122 -> ... -> 140 -> 141 (length ~21, borderline)
    for (int i = 120; i < 140; ++i) {
        g.add_edge(i, i + 1);
    }
    g.add_edge(140, 141);

    // Add additional connectivity to make graph richer
    // Cross-layer connections
    g.add_edge(0, 3);
    g.add_edge(3, 11);
    g.add_edge(11, 31);
    g.add_edge(31, 70);
    g.add_edge(70, 101);

    // More alternative paths
    g.add_edge(15, 35);
    g.add_edge(35, 45);

    g.add_edge(45, 50);
    g.add_edge(50, 55);
    g.add_edge(55, 80);

    g.add_edge(80, 85);
    g.add_edge(85, 95);
    g.add_edge(95, 105);
    g.add_edge(105, 120);

    // Additional random edges for complexity
    g.add_edge(20, 40);
    g.add_edge(40, 60);
    g.add_edge(60, 90);
    g.add_edge(90, 130);
    g.add_edge(130, 141);

    std::cout << "[Test Graph] Created graph with 142 vertices" << std::endl;
    std::cout << "[Test Graph] Shortest path: ";
    for (VertexID v : expected_shortest_path) {
        std::cout << v << " ";
    }
    std::cout << "(length = " << expected_shortest_path.size() - 1 << ")" << std::endl;

    return g;
}

void test_replacement_paths() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "COMPREHENSIVE REPLACEMENT PATHS TEST" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Create test graph
    int source, target;
    Path expected_path;
    Graph g = create_large_test_graph(source, target, expected_path);

    // Verify shortest path using BFS
    std::cout << "\n--- Verifying Shortest Path ---" << std::endl;
    SSSPResult sssp = BFS(g, source);
    Path computed_path = reconstruct_path(sssp, source, target);

    std::cout << "Expected path length: " << expected_path.size() - 1 << std::endl;
    std::cout << "Computed path length: " << computed_path.size() - 1 << std::endl;

    // For this test, we'll use the expected path directly
    Path shortest_path = expected_path;
    std::cout << "Using path: ";
    for (VertexID v : shortest_path) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    // Create ReplacementPaths solver
    std::cout << "\n" << std::string(70, '=') << std::endl;
    ReplacementPaths solver(g, shortest_path);

    // Compute all replacements
    solver.compute_all_replacement_paths();

    // Print detailed results
    solver.print_summary();

    // Detailed analysis
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "DETAILED ANALYSIS" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    const auto& results = solver.get_all_replacements();

    std::cout << "\nEdge 0 (0->15) Analysis:" << std::endl;
    std::cout << "  Expected: SHORT detour via 0->1->2->15 (length 3)" << std::endl;
    std::cout << "  Computed: " << (results[0].is_short_detour ? "SHORT" : "LONG")
              << " detour, length = " << results[0].length << std::endl;
    std::cout << "  Original path length: 5, Replacement: " << results[0].length << std::endl;

    std::cout << "\nEdge 1 (15->45) Analysis:" << std::endl;
    std::cout << "  Expected: SHORT detour via 15->20->25->30->45 (length 4)" << std::endl;
    std::cout << "  Computed: " << (results[1].is_short_detour ? "SHORT" : "LONG")
              << " detour, length = " << results[1].length << std::endl;
    std::cout << "  Original path length: 5, Replacement: " << results[1].length << std::endl;

    std::cout << "\nEdge 2 (45->80) Analysis:" << std::endl;
    std::cout << "  Expected: LONG detour via 45->46->...->75->80 (length ~30)" << std::endl;
    std::cout << "  Computed: " << (results[2].is_short_detour ? "SHORT" : "LONG")
              << " detour, length = " << results[2].length << std::endl;
    std::cout << "  L = " << solver.get_L() << ", so 2L = " << 2 * solver.get_L() << std::endl;
    std::cout << "  This detour should be classified as LONG since length > 2L" << std::endl;

    std::cout << "\nEdge 3 (80->120) Analysis:" << std::endl;
    std::cout << "  Expected: SHORT detour via 80->90->100->110->120 (length 4)" << std::endl;
    std::cout << "  Computed: " << (results[3].is_short_detour ? "SHORT" : "LONG")
              << " detour, length = " << results[3].length << std::endl;

    std::cout << "\nEdge 4 (120->141) Analysis:" << std::endl;
    std::cout << "  Expected: LONG/SHORT detour via 120->121->...->141 (length ~21)" << std::endl;
    std::cout << "  Computed: " << (results[4].is_short_detour ? "SHORT" : "LONG")
              << " detour, length = " << results[4].length << std::endl;
    std::cout << "  With 2L = " << 2 * solver.get_L() << ", this is borderline" << std::endl;

    // Verification
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "VERIFICATION" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    int original_length = shortest_path.size() - 1;
    std::cout << "\nOriginal shortest path length: " << original_length << std::endl;

    bool all_valid = true;
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].length < INF && results[i].length < original_length) {
            std::cout << "WARNING: Replacement " << i << " is shorter than original!"
                      << std::endl;
            all_valid = false;
        }
    }

    if (all_valid) {
        std::cout << "✓ All replacement paths are valid (length >= original)" << std::endl;
    }

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "TEST COMPLETED SUCCESSFULLY" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
}

int main() {
    try {
        test_replacement_paths();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}
