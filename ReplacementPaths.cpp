#include "ReplacementPaths.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <random>
#include <cmath>

ReplacementPaths::ReplacementPaths(const Graph& G, const Path& P)
    : graph(G), shortest_path(P), short_detour_module(G, P) {

    // Calculate L = ceil(sqrt(n))
    int n = G.get_vertex_count();
    L = static_cast<int>(std::ceil(std::sqrt(n)));

    // Initialize results vector
    int num_edges = P.size() - 1;
    replacement_results.resize(num_edges);

    std::cout << "[ReplacementPaths] Initialized with n=" << n
              << ", L=" << L << ", path_length=" << P.size() << std::endl;
}

std::vector<int> ReplacementPaths::bfs_excluding_path(
    VertexID start,
    const std::set<std::pair<VertexID, VertexID>>& path_edges,
    bool use_reverse) const {

    int n = graph.get_vertex_count();
    std::vector<int> dist(n, INF);
    dist[start] = 0;

    std::queue<VertexID> q;
    q.push(start);

    // Get appropriate adjacency list
    Graph g_for_bfs = use_reverse ? graph.create_reversed_graph() : graph;
    const AdjacencyList& adj = g_for_bfs.get_adj_list();

    while (!q.empty()) {
        VertexID u = q.front();
        q.pop();

        if (adj.find(u) == adj.end()) continue;

        for (VertexID v : adj.at(u)) {
            // Check if edge is forbidden
            bool is_forbidden = false;
            if (use_reverse) {
                // In reverse graph, we traverse u->v which is v->u in original
                if (path_edges.count({v, u})) {
                    is_forbidden = true;
                }
            } else {
                if (path_edges.count({u, v})) {
                    is_forbidden = true;
                }
            }

            if (is_forbidden) continue;

            if (dist[v] == INF) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }

    return dist;
}

std::map<int, int> ReplacementPaths::solve_long_detours() {
    int n = graph.get_vertex_count();
    int path_len = shortest_path.size();

    // Build path edges set for exclusion
    std::set<std::pair<VertexID, VertexID>> path_edges;
    for (size_t i = 0; i < shortest_path.size() - 1; ++i) {
        path_edges.insert({shortest_path[i], shortest_path[i+1]});
    }

    // Initialize results
    std::map<int, int> best_replacement_len;
    for (int i = 0; i < path_len - 1; ++i) {
        best_replacement_len[i] = INF;
    }

    // Random sampling (Lemma 2.4 from paper)
    double c = 4.0; // Constant for probability
    double probability = (c * std::log(n)) / L;
    probability = std::min(probability, 1.0);

    std::vector<VertexID> R; // Sample set
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int v = 0; v < n; ++v) {
        if (dis(gen) < probability) {
            R.push_back(v);
        }
    }

    std::cout << "[LongDetour] Sampled " << R.size() << " vertices (probability="
              << probability << ")" << std::endl;

    // Process each sampled vertex
    for (VertexID r : R) {
        // Forward BFS from r in G-P
        std::vector<int> dist_from_r = bfs_excluding_path(r, path_edges, false);

        // Backward BFS to r in G-P (using reverse graph)
        std::vector<int> dist_to_r = bfs_excluding_path(r, path_edges, true);

        // Compute costs for entering and exiting through sampled vertex
        std::vector<int> val_in(path_len, INF);
        std::vector<int> val_out(path_len, INF);

        for (int a = 0; a < path_len; ++a) {
            VertexID u_a = shortest_path[a];
            if (u_a < (int)dist_to_r.size() && dist_to_r[u_a] != INF) {
                val_in[a] = a + dist_to_r[u_a];
            }
        }

        for (int b = 0; b < path_len; ++b) {
            VertexID u_b = shortest_path[b];
            if (u_b < (int)dist_from_r.size() && dist_from_r[u_b] != INF) {
                val_out[b] = dist_from_r[u_b] + (path_len - 1 - b);
            }
        }

        // Compute prefix/suffix minimums for efficient query
        std::vector<int> prefix_min_in(path_len, INF);
        int current_min = INF;
        for (int i = 0; i < path_len; ++i) {
            current_min = std::min(current_min, val_in[i]);
            prefix_min_in[i] = current_min;
        }

        std::vector<int> suffix_min_out(path_len, INF);
        current_min = INF;
        for (int i = path_len - 1; i >= 0; --i) {
            current_min = std::min(current_min, val_out[i]);
            suffix_min_out[i] = current_min;
        }

        // Update best replacement for each edge
        for (int i = 0; i < path_len - 1; ++i) {
            int best_a_cost = prefix_min_in[i];
            int best_b_cost = suffix_min_out[i + 1];

            if (best_a_cost != INF && best_b_cost != INF) {
                int total = best_a_cost + best_b_cost;
                best_replacement_len[i] = std::min(best_replacement_len[i], total);
            }
        }
    }

    return best_replacement_len;
}

void ReplacementPaths::compute_all_replacement_paths() {
    std::cout << "\n=== Computing Replacement Paths ===" << std::endl;
    std::cout << "Graph: n=" << graph.get_vertex_count()
              << ", Shortest path length=" << shortest_path.size() - 1 << std::endl;
    std::cout << "Threshold L=" << L << " (short detours: length <= " << L << ")" << std::endl;

    // Step 1: Compute RD-Table for short detours
    std::cout << "\n--- Step 1: Computing Short Detours (RD-Table) ---" << std::endl;
    short_detour_module.compute_RD_table();

    // Step 2: Solve long detours using sampling
    std::cout << "\n--- Step 2: Computing Long Detours (Sampling) ---" << std::endl;
    std::map<int, int> long_detour_lengths = solve_long_detours();

    // Step 3: For each edge, choose best between short and long detour
    std::cout << "\n--- Step 3: Selecting Best Replacement Paths ---" << std::endl;

    int num_short = 0;
    int num_long = 0;
    int num_none = 0;

    int path_len = shortest_path.size();

    // Define Priority Queue OUTSIDE the main loop for sliding window optimization
    // Min-PQ stores {value, {start_index_a, jump_j}}
    using PQElement = std::pair<int, std::pair<int, int>>;
    std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> pq;

    // Iterate through edges to be removed (i)
    for (int i = 0; i < (int)shortest_path.size() - 1; ++i) {

        // A. Add NEW detours that start at the current index 'i'
        //    These detours start at i (a=i) and land at b = i + j
        for (int j = 1; j <= L; j++) {
            int a = i;
            int b = a + j;
            if (b >= path_len) continue;

            int detour_cost = short_detour_module.get_replacement_distance(a, j);
            if (detour_cost != INF) {
                // Total cost = a + detour + suffix_len
                int total_cost = a + detour_cost + (path_len - 1 - b);
                pq.push({total_cost, {a, j}});
            }
        }

        // B. Remove INVALID detours (Lazy Deletion)
        //    A detour is invalid if it lands at or before the removed edge 'i' (b <= i)
        //    because it doesn't actually bypass the break.
        while (!pq.empty()) {
            int a = pq.top().second.first;
            int j = pq.top().second.second;
            int b = a + j;

            // If the detour lands after the break (b > i), it is valid.
            // Since we also add detours sequentially, 'a' is always <= i.
            if (b > i) {
                break;
            }

            // Otherwise, this detour ends too early. Remove it.
            pq.pop();
        }

        // C. The top of the PQ is now the best valid short detour
        int short_len = INF;
        if (!pq.empty()) {
            short_len = pq.top().first;
        }

        // Get long detour length
        int long_len = long_detour_lengths[i];

        // Choose the better option
        if (short_len <= long_len) {
            if (short_len < INF) {
                Path short_path = short_detour_module.shortRepPath(i);
                replacement_results[i].path = short_path;
                replacement_results[i].length = short_len;
                replacement_results[i].is_short_detour = true;
                num_short++;
            } else {
                num_none++;
            }
        } else {
            if (long_len < INF) {
                // For long detour, we have the length but not the actual path
                // In practice, you would reconstruct it from parent pointers
                replacement_results[i].length = long_len;
                replacement_results[i].is_short_detour = false;
                num_long++;
            } else {
                num_none++;
            }
        }
    }

    std::cout << "\nResults Summary:" << std::endl;
    std::cout << "  Short detours used: " << num_short << std::endl;
    std::cout << "  Long detours used: " << num_long << std::endl;
    std::cout << "  No replacement found: " << num_none << std::endl;
}

const ReplacementPaths::ReplacementResult&
ReplacementPaths::get_replacement(int edge_index) const {
    if (edge_index < 0 || edge_index >= (int)replacement_results.size()) {
        static ReplacementResult invalid;
        return invalid;
    }
    return replacement_results[edge_index];
}

void ReplacementPaths::print_summary() const {
    std::cout << "\n=== Replacement Paths Summary ===" << std::endl;
    std::cout << "Edge | Removed Edge    | Repl. Length | Type" << std::endl;
    std::cout << "-----+-----------------+--------------+----------" << std::endl;

    for (size_t i = 0; i < replacement_results.size(); ++i) {
        VertexID u = shortest_path[i];
        VertexID v = shortest_path[i+1];
        const auto& result = replacement_results[i];

        std::cout << std::setw(4) << i << " | "
                  << "(" << std::setw(3) << u << " -> " << std::setw(3) << v << ")  | ";

        if (result.length == INF) {
            std::cout << std::setw(12) << "INF" << " | ";
            std::cout << "NONE";
        } else {
            std::cout << std::setw(12) << result.length << " | ";
            std::cout << (result.is_short_detour ? "SHORT" : "LONG");
        }

        std::cout << std::endl;
    }
}
