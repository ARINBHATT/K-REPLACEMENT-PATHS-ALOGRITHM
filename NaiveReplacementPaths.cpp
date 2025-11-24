#include "NaiveReplacementPaths.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <iomanip>

NaiveReplacementPaths::NaiveReplacementPaths(const Graph& G, const Path& P)
    : graph(G), shortest_path(P) {

    int num_edges = P.size() - 1;
    replacement_results.resize(num_edges);

    std::cout << "[NaiveReplacementPaths] Initialized for path of length "
              << num_edges << std::endl;
}

void NaiveReplacementPaths::compute_all_replacement_paths() {
    std::cout << "\n=== Computing Replacement Paths (Naive Algorithm) ===" << std::endl;
    std::cout << "Method: Remove each edge and run BFS" << std::endl;
    std::cout << "Time Complexity: O(mn) where m=edges, n=vertices" << std::endl;

    VertexID source = shortest_path.front();
    VertexID target = shortest_path.back();

    // For each edge on the shortest path
    for (size_t i = 0; i < shortest_path.size() - 1; ++i) {
        VertexID u = shortest_path[i];
        VertexID v = shortest_path[i + 1];

        std::cout << "\nEdge " << i << ": Removing (" << u << " -> " << v << ")" << std::endl;

        // Create a copy of the graph and remove this edge
        Graph g_temp = graph;
        g_temp.remove_edge(u, v);

        // Run BFS from source to target in modified graph
        SSSPResult sssp = BFS(g_temp, source);

        // Reconstruct the replacement path
        Path replacement = reconstruct_path(sssp, source, target);

        if (replacement.empty() || replacement.back() != target) {
            // No path found
            replacement_results[i].length = INF;
            std::cout << "  Result: No replacement path found" << std::endl;
        } else {
            replacement_results[i].length = replacement.size() - 1;
            replacement_results[i].path = replacement;
            std::cout << "  Result: Replacement path length = "
                      << replacement_results[i].length << std::endl;
        }
    }

    std::cout << "\nNaive algorithm completed." << std::endl;
}

const NaiveReplacementPaths::ReplacementResult&
NaiveReplacementPaths::get_replacement(int edge_index) const {
    if (edge_index < 0 || edge_index >= (int)replacement_results.size()) {
        static ReplacementResult invalid;
        return invalid;
    }
    return replacement_results[edge_index];
}

void NaiveReplacementPaths::print_summary() const {
    std::cout << "\n=== Naive Algorithm Results Summary ===" << std::endl;
    std::cout << "Edge | Removed Edge    | Repl. Length" << std::endl;
    std::cout << "-----+-----------------+-------------" << std::endl;

    for (size_t i = 0; i < replacement_results.size(); ++i) {
        VertexID u = shortest_path[i];
        VertexID v = shortest_path[i+1];
        const auto& result = replacement_results[i];

        std::cout << std::setw(4) << i << " | "
                  << "(" << std::setw(3) << u << " -> " << std::setw(3) << v << ")  | ";

        if (result.length == INF) {
            std::cout << std::setw(12) << "INF";
        } else {
            std::cout << std::setw(12) << result.length;
        }

        std::cout << std::endl;
    }
}
