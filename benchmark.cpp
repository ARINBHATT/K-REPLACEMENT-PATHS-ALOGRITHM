#include "ReplacementPaths.h"
#include "NaiveReplacementPaths.h"
#include "common/CommonGraph.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <iomanip>
#include <vector>

using namespace std::chrono;

/**
 * Generate a random graph with specified properties
 */
Graph generate_random_graph(int n, double edge_probability, int& source, int& target, Path& shortest_path) {
    Graph g;

    // Initialize all vertices
    for (int i = 0; i < n; ++i) {
        g.add_edge(i, i);
        g.remove_edge(i, i);
    }

    // Create a guaranteed shortest path
    int path_length = std::min(n / 10 + 5, n - 1);
    std::vector<int> path_vertices;

    std::random_device rd;
    std::mt19937 gen(rd());

    // Select random vertices for the path
    std::vector<int> available(n);
    std::iota(available.begin(), available.end(), 0);
    std::shuffle(available.begin(), available.end(), gen);

    for (int i = 0; i < path_length; ++i) {
        path_vertices.push_back(available[i]);
    }

    // Add edges for the shortest path
    for (size_t i = 0; i < path_vertices.size() - 1; ++i) {
        g.add_edge(path_vertices[i], path_vertices[i+1]);
    }

    source = path_vertices[0];
    target = path_vertices.back();
    shortest_path = path_vertices;

    // Add random edges (avoid creating shorter paths)
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (u != v && dis(gen) < edge_probability) {
                // Only add if it doesn't create a shortcut in the shortest path
                bool is_path_edge = false;
                for (size_t i = 0; i < path_vertices.size() - 1; ++i) {
                    if (u == path_vertices[i] && v == path_vertices[i+1]) {
                        is_path_edge = true;
                        break;
                    }
                }

                // Add detour edges (skip ahead on path by 2-5 positions)
                if (!is_path_edge) {
                    g.add_edge(u, v);
                }
            }
        }
    }

    return g;
}

/**
 * Generate a grid graph with guaranteed paths
 */
Graph generate_grid_graph(int rows, int cols, int& source, int& target, Path& shortest_path) {
    Graph g;
    int n = rows * cols;

    // Lambda to convert (row, col) to vertex ID
    auto to_id = [cols](int r, int c) { return r * cols + c; };

    // Initialize vertices
    for (int i = 0; i < n; ++i) {
        g.add_edge(i, i);
        g.remove_edge(i, i);
    }

    // Add grid edges
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int id = to_id(r, c);

            // Right edge
            if (c + 1 < cols) {
                g.add_edge(id, to_id(r, c+1));
            }

            // Down edge
            if (r + 1 < rows) {
                g.add_edge(id, to_id(r+1, c));
            }

            // Diagonal (for more detours)
            if (r + 1 < rows && c + 1 < cols) {
                g.add_edge(id, to_id(r+1, c+1));
            }
        }
    }

    // Source: top-left, Target: bottom-right
    source = to_id(0, 0);
    target = to_id(rows-1, cols-1);

    // Find shortest path using BFS
    SSSPResult sssp = BFS(g, source);
    shortest_path = reconstruct_path(sssp, source, target);

    return g;
}

/**
 * Benchmark structure to store results
 */
struct BenchmarkResult {
    int n_vertices;
    int n_edges;
    int path_length;
    double naive_time_ms;
    double optimized_time_ms;
    double speedup;
    bool naive_completed;
    bool optimized_completed;
};

/**
 * Run benchmark on a single graph
 */
BenchmarkResult run_benchmark(Graph& g, const Path& shortest_path, bool run_naive = true) {
    BenchmarkResult result;
    result.n_vertices = g.get_vertex_count();
    result.path_length = shortest_path.size() - 1;

    // Count edges
    int edge_count = 0;
    const AdjacencyList& adj = g.get_adj_list();
    for (const auto& [u, neighbors] : adj) {
        edge_count += neighbors.size();
    }
    result.n_edges = edge_count;

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Benchmark: n=" << result.n_vertices
              << ", m=" << result.n_edges
              << ", path_len=" << result.path_length << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Benchmark optimized algorithm
    std::cout << "\n--- Running Optimized Algorithm ---" << std::endl;
    auto start = high_resolution_clock::now();

    try {
        ReplacementPaths optimized(g, shortest_path);
        optimized.compute_all_replacement_paths();
        auto end = high_resolution_clock::now();
        result.optimized_time_ms = duration_cast<microseconds>(end - start).count() / 1000.0;
        result.optimized_completed = true;
        std::cout << "Optimized Time: " << result.optimized_time_ms << " ms" << std::endl;
    } catch (...) {
        result.optimized_completed = false;
        result.optimized_time_ms = -1;
        std::cout << "Optimized algorithm failed!" << std::endl;
    }

    // Benchmark naive algorithm (skip for large graphs)
    if (run_naive && result.n_vertices <= 500) {
        std::cout << "\n--- Running Naive Algorithm ---" << std::endl;
        start = high_resolution_clock::now();

        try {
            NaiveReplacementPaths naive(g, shortest_path);
            naive.compute_all_replacement_paths();
            auto end = high_resolution_clock::now();
            result.naive_time_ms = duration_cast<microseconds>(end - start).count() / 1000.0;
            result.naive_completed = true;
            std::cout << "Naive Time: " << result.naive_time_ms << " ms" << std::endl;
        } catch (...) {
            result.naive_completed = false;
            result.naive_time_ms = -1;
            std::cout << "Naive algorithm failed!" << std::endl;
        }
    } else {
        result.naive_completed = false;
        result.naive_time_ms = -1;
        std::cout << "\n--- Skipping Naive Algorithm (graph too large) ---" << std::endl;
    }

    // Calculate speedup
    if (result.naive_completed && result.optimized_completed && result.naive_time_ms > 0) {
        result.speedup = result.naive_time_ms / result.optimized_time_ms;
        std::cout << "\nSpeedup: " << result.speedup << "x" << std::endl;
    } else {
        result.speedup = -1;
    }

    return result;
}

/**
 * Save results to CSV
 */
void save_results_csv(const std::vector<BenchmarkResult>& results, const std::string& filename) {
    std::ofstream file(filename);
    file << "n_vertices,n_edges,path_length,naive_time_ms,optimized_time_ms,speedup\n";

    for (const auto& r : results) {
        file << r.n_vertices << ","
             << r.n_edges << ","
             << r.path_length << ","
             << r.naive_time_ms << ","
             << r.optimized_time_ms << ","
             << r.speedup << "\n";
    }

    file.close();
    std::cout << "\nResults saved to: " << filename << std::endl;
}

int main() {
    std::vector<BenchmarkResult> results;

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << " REPLACEMENT PATHS BENCHMARK SUITE\n";
    std::cout << "========================================\n";
    std::cout << "\nComparing:\n";
    std::cout << "  1. Naive Algorithm: O(mn) - Remove edge + BFS\n";
    std::cout << "  2. Roditty-Zwick: O(m√n log n) - Short+Long Detours\n\n";

    // Test 1: Small random graphs
    std::cout << "\n### TEST SUITE 1: Random Graphs (Small to Medium) ###\n";
    std::vector<int> sizes = {50, 100, 200, 300, 400, 500};

    for (int n : sizes) {
        int source, target;
        Path shortest_path;
        double edge_prob = 0.05;  // Sparse graph

        Graph g = generate_random_graph(n, edge_prob, source, target, shortest_path);
        BenchmarkResult result = run_benchmark(g, shortest_path, true);
        results.push_back(result);
    }

    // Test 2: Grid graphs
    std::cout << "\n### TEST SUITE 2: Grid Graphs ###\n";
    std::vector<std::pair<int, int>> grid_sizes = {
        {10, 10},   // 100 vertices
        {15, 15},   // 225 vertices
        {20, 20},   // 400 vertices
        {25, 25}    // 625 vertices (skip naive)
    };

    for (const auto& [rows, cols] : grid_sizes) {
        int source, target;
        Path shortest_path;

        Graph g = generate_grid_graph(rows, cols, source, target, shortest_path);
        bool run_naive = (rows * cols <= 500);
        BenchmarkResult result = run_benchmark(g, shortest_path, run_naive);
        results.push_back(result);
    }

    // Test 3: Large graphs (optimized only)
    std::cout << "\n### TEST SUITE 3: Large Graphs (Optimized Only) ###\n";
    std::vector<int> large_sizes = {1000, 2000, 5000};

    for (int n : large_sizes) {
        int source, target;
        Path shortest_path;
        double edge_prob = 0.01;  // Very sparse for large graphs

        Graph g = generate_random_graph(n, edge_prob, source, target, shortest_path);
        BenchmarkResult result = run_benchmark(g, shortest_path, false);
        results.push_back(result);
    }

    // Save results
    save_results_csv(results, "benchmark_results.csv");

    // Print summary table
    std::cout << "\n" << std::string(90, '=') << std::endl;
    std::cout << "BENCHMARK SUMMARY" << std::endl;
    std::cout << std::string(90, '=') << std::endl;
    std::cout << std::setw(10) << "n"
              << std::setw(10) << "m"
              << std::setw(12) << "Path Len"
              << std::setw(15) << "Naive (ms)"
              << std::setw(15) << "Optimized (ms)"
              << std::setw(12) << "Speedup"
              << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (const auto& r : results) {
        std::cout << std::setw(10) << r.n_vertices
                  << std::setw(10) << r.n_edges
                  << std::setw(12) << r.path_length;

        if (r.naive_completed) {
            std::cout << std::setw(15) << std::fixed << std::setprecision(2) << r.naive_time_ms;
        } else {
            std::cout << std::setw(15) << "SKIPPED";
        }

        if (r.optimized_completed) {
            std::cout << std::setw(15) << std::fixed << std::setprecision(2) << r.optimized_time_ms;
        } else {
            std::cout << std::setw(15) << "FAILED";
        }

        if (r.speedup > 0) {
            std::cout << std::setw(12) << std::fixed << std::setprecision(2) << r.speedup << "x";
        } else {
            std::cout << std::setw(12) << "N/A";
        }

        std::cout << std::endl;
    }

    std::cout << "\nBenchmark completed! Results saved to benchmark_results.csv\n";

    return 0;
}
