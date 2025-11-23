#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <set>
#include <map>
#include <random>
#include <limits>

// Use a large number for Infinity, but safe enough to add without overflow
const int INF = 1e9;

// -----------------------------------------------------------------------------
// Data Structures
// -----------------------------------------------------------------------------

struct Graph {
    int n;
    bool directed;
    std::vector<std::vector<int>> adj;
    std::vector<std::vector<int>> rev_adj; // Reverse adjacency for backward BFS

    Graph(int nodes, bool is_directed = true) : n(nodes), directed(is_directed) {
        adj.resize(n);
        rev_adj.resize(n);
    }

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        if (directed) {
            rev_adj[v].push_back(u);
        } else {
            adj[v].push_back(u); // For undirected, add edge both ways
            rev_adj[v].push_back(u);
            rev_adj[u].push_back(v);
        }
    }
};

// -----------------------------------------------------------------------------
// Helper Functions
// -----------------------------------------------------------------------------

/**
 * Performs a BFS from start_node, explicitly ignoring edges that are part of the shortest path.
 * * @param graph The graph structure
 * @param start_node The source of the BFS
 * @param path_edges A set of pairs representing edges on the shortest path to avoid
 * @param reverse If true, traverses the graph using reverse edges (rev_adj)
 * @return A vector containing distances from start_node to all other nodes
 */
std::vector<int> bfs_excluding_path(const Graph& graph, int start_node, 
                                    const std::set<std::pair<int, int>>& path_edges, 
                                    bool reverse) {
    std::vector<int> dist(graph.n, INF);
    dist[start_node] = 0;
    
    std::queue<int> q;
    q.push(start_node);

    const auto& adjacency = reverse ? graph.rev_adj : graph.adj;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        int current_dist = dist[u];

        for (int v : adjacency[u]) {
            // Check if this edge is on the forbidden path
            // If reverse=true, we are effectively traversing v->u in the original graph,
            // but physically traversing u->v in the reverse graph.
            // The set path_edges stores original direction edges (u, v).
            
            bool is_forbidden = false;
            if (reverse) {
                // We are moving u->v in reverse graph, which corresponds to v->u in original.
                if (path_edges.count({v, u})) is_forbidden = true;
            } else {
                // Standard forward traversal
                if (path_edges.count({u, v})) is_forbidden = true;
            }

            if (is_forbidden) continue;

            if (dist[v] == INF) {
                dist[v] = current_dist + 1;
                q.push(v);
            }
        }
    }
    return dist;
}

// -----------------------------------------------------------------------------
// Core Logic: Long Detour Algorithm (Section 2.2)
// -----------------------------------------------------------------------------

/**
 * Implements the Long Detour part of the Roditty-Zwick algorithm.
 * * @param graph The graph object
 * @param s Source vertex
 * @param t Target vertex
 * @param shortest_path The vector of vertices representing the shortest path [s, ..., t]
 * @param L The threshold length
 * @return A map where Key is edge index (0 to |P|-2), Value is the replacement path length
 */
std::map<int, int> solve_long_detours(const Graph& graph, int s, int t, 
                                      const std::vector<int>& shortest_path, int L) {
    int n = graph.n;
    int path_len = shortest_path.size();
    
    // 1. Identify edges on the shortest path for fast lookup during BFS
    std::set<std::pair<int, int>> path_edges;
    for (size_t i = 0; i < shortest_path.size() - 1; ++i) {
        path_edges.insert({shortest_path[i], shortest_path[i+1]});
    }

    // Initialize results with INF
    std::map<int, int> best_replacement_len;
    for (int i = 0; i < path_len - 1; ++i) {
        best_replacement_len[i] = INF;
    }

    // 2. Random Sampling (Lemma 2.4)
    // Probability p = (c * ln n) / L
    double c = 5.0; // Constant factor
    double probability = 1.0;
    if (L > 0) {
        probability = (c * std::log(n)) / L;
    }
    
    std::vector<int> R; // The sample set
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int v = 0; v < n; ++v) {
        if (dis(gen) < probability) {
            R.push_back(v);
        }
    }
    
    std::cout << "[Log] Sampled " << R.size() << " vertices for long detour search." << std::endl;

    // 3. Process each sampled vertex
    for (int r : R) {
        // A. Forward BFS from r (calculates dist(r, v) in G-P)
        std::vector<int> dist_from_r = bfs_excluding_path(graph, r, path_edges, false);
        
        // B. Backward BFS to r (calculates dist(u, r) in G-P)
        std::vector<int> dist_to_r = bfs_excluding_path(graph, r, path_edges, true);

        // C. Calculate valid path costs
        // We want to find a detour u_a -> ... -> r -> ... -> u_b
        // total_cost = index(a) + dist(u_a, r) + dist(r, u_b) + (len - 1 - index(b))
        
        std::vector<int> val_in(path_len, INF);
        std::vector<int> val_out(path_len, INF);

        // Precompute costs for entering the detour at 'a'
        for (int a = 0; a < path_len; ++a) {
            int u_a = shortest_path[a];
            if (dist_to_r[u_a] != INF) {
                val_in[a] = a + dist_to_r[u_a];
            }
        }

        // Precompute costs for exiting the detour at 'b'
        for (int b = 0; b < path_len; ++b) {
            int u_b = shortest_path[b];
            if (dist_from_r[u_b] != INF) {
                val_out[b] = dist_from_r[u_b] + (path_len - 1 - b);
            }
        }

        // D. Efficiently update best replacement for every cut
        // For an edge index 'i' (edge between path[i] and path[i+1]),
        // we need min(val_in[a] + val_out[b]) such that a <= i < b.
        
        // We calculate prefix minimums for val_in
        std::vector<int> prefix_min_in(path_len, INF);
        int current_min = INF;
        for (int i = 0; i < path_len; ++i) {
            if (val_in[i] < current_min) current_min = val_in[i];
            prefix_min_in[i] = current_min;
        }

        // We calculate suffix minimums for val_out
        std::vector<int> suffix_min_out(path_len, INF);
        current_min = INF;
        for (int i = path_len - 1; i >= 0; --i) {
            if (val_out[i] < current_min) current_min = val_out[i];
            suffix_min_out[i] = current_min;
        }

        // Combine: for edge i, valid 'a' is in [0...i], valid 'b' is in [i+1...end]
        for (int i = 0; i < path_len - 1; ++i) {
            int best_a_cost = prefix_min_in[i];
            int best_b_cost = suffix_min_out[i+1]; // b > i implies b >= i+1

            if (best_a_cost != INF && best_b_cost != INF) {
                int total = best_a_cost + best_b_cost;
                if (total < best_replacement_len[i]) {
                    best_replacement_len[i] = total;
                }
            }
        }
    }

    return best_replacement_len;
}

// -----------------------------------------------------------------------------
// Main Driver / Example Usage
// -----------------------------------------------------------------------------

int main() {
    // Example Configuration
    // Graph based on the simple scenario discussed
    // 0 -> 1 -> 2 -> 3 (Shortest Path)
    // Edge 1->2 has a LONG detour: 1 -> 4 -> 5 -> 6 -> 2
    
    int n = 7;
    Graph g(n, true); // 7 nodes, directed

    // Shortest Path Edges
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 3);

    // Long detour edges connecting 1 to 2
    g.add_edge(1, 4);
    g.add_edge(4, 5);
    g.add_edge(5, 6);
    g.add_edge(6, 2);

    std::vector<int> shortest_path = {0, 1, 2, 3};
    
    // Threshold L = sqrt(n)
    int L = static_cast<int>(std::sqrt(n)); 
    
    std::cout << "Graph Size: " << n << std::endl;
    std::cout << "Threshold L: " << L << std::endl;
    std::cout << "Shortest Path: ";
    for(int u : shortest_path) std::cout << u << " ";
    std::cout << std::endl;

    // Run Long Detour Algorithm
    std::map<int, int> results = solve_long_detours(g, 0, 3, shortest_path, L);

    std::cout << "\n--- Results (Long Detour Search) ---" << std::endl;
    std::cout << "Edge Idx | Edge Removed | Replacement Len" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    for (size_t i = 0; i < shortest_path.size() - 1; ++i) {
        int u = shortest_path[i];
        int v = shortest_path[i+1];
        int len = results[i];

        std::cout << i << "        | (" << u << "->" << v << ")      | ";
        if (len == INF) {
            std::cout << "Inf" << std::endl;
        } else {
            std::cout << len << std::endl;
        }
    }
    
    std::cout << "\nNote: 'Inf' means no detour > L was found." << std::endl;
    std::cout << "      (A shorter detour might exist, handled by the other module)." << std::endl;

    return 0;
}