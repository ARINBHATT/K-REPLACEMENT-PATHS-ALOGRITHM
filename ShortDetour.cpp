#include "ShortDetour.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <algorithm>

// ============================================================================
// ShortDetour Implementation
// ============================================================================

ShortDetour::ShortDetour(const Graph& G, const Path& P)
    : original_graph(G), shortest_path(P) {

    // Compute L = ceil(sqrt(n))
    int n = G.get_vertex_count();
    L = static_cast<int>(std::ceil(std::sqrt(n)));

    // Create G - P (graph with shortest path edges removed)
    graph_minus_path = G.create_graph_minus_path(P);

    // Initialize RD-table: RD_table[i][j] represents replacement distance
    // from P[i] to P[i+j] for j in [1, L]
    int path_len = P.size();
    RD_table.resize(path_len, std::vector<int>(L + 1, INF));
}

void ShortDetour::compute_RD_table() {
    std::cout << "[ShortDetour] Computing RD-Table with L = " << L << std::endl;

    // For each offset b from 0 to 2L-1
    for (int b = 0; b < 2 * L; b++) {
        // Create auxiliary graph and fill corresponding RD-table entries
        AuxiliaryGraph aux_graph(graph_minus_path, shortest_path, b, L);
        aux_graph.fill_RD_table(RD_table, shortest_path, b, L);
    }

    std::cout << "[ShortDetour] RD-Table computation complete" << std::endl;
}

int ShortDetour::get_replacement_distance(int i, int j) const {
    if (i < 0 || i >= (int)RD_table.size() || j < 1 || j > L) {
        return INF;
    }
    if (i + j >= (int)shortest_path.size()) {
        return INF;
    }
    return RD_table[i][j];
}

Path ShortDetour::shortRepPath(int edge_index) {
    // edge_index represents the edge (P[edge_index], P[edge_index+1]) that is removed

    if (edge_index < 0 || edge_index >= (int)shortest_path.size() - 1) {
        return {}; // Invalid edge index
    }

    int path_len = shortest_path.size();
    int best_distance = INF;
    int best_start = -1;
    int best_jump = -1;

    // Find the best short detour that bypasses the removed edge
    // We need a detour from P[a] to P[b] where a <= edge_index < b
    // and the detour length is <= 2L

    for (int a = 0; a <= edge_index; a++) {
        for (int j = 1; j <= L; j++) {
            int b = a + j;

            // Check if this detour bypasses the removed edge
            if (b <= edge_index || b >= path_len) {
                continue; // Doesn't bypass the edge or out of bounds
            }

            // Calculate total replacement path length
            // Length = a (edges before detour) + RD[a][j] (detour) + (path_len - 1 - b) (edges after detour)
            int detour_cost = get_replacement_distance(a, j);

            if (detour_cost == INF) {
                continue; // No such detour exists
            }

            int total_length = a + detour_cost + (path_len - 1 - b);

            if (total_length < best_distance) {
                best_distance = total_length;
                best_start = a;
                best_jump = j;
            }
        }
    }

    // If no short detour found, return empty path
    if (best_start == -1) {
        return {};
    }

    // Reconstruct the replacement path
    // Path consists of: P[0..a] + detour(P[a] -> P[a+j]) + P[a+j..end]

    // For now, we return an indicator path showing which detour was chosen
    // In a full implementation, you would actually reconstruct the detour path through G-P
    // This would require storing parent pointers from the Dijkstra computation

    Path replacement;

    // Add path prefix P[0..best_start]
    for (int i = 0; i <= best_start; i++) {
        replacement.push_back(shortest_path[i]);
    }

    // Note: The actual detour path from P[best_start] to P[best_start + best_jump]
    // would need to be reconstructed from BFS/Dijkstra parent pointers
    // For this implementation, we'll add a marker showing the jump

    // Skip to the end of detour
    int detour_end = best_start + best_jump;

    // Add path suffix P[detour_end..end]
    for (int i = detour_end; i < path_len; i++) {
        replacement.push_back(shortest_path[i]);
    }

    return replacement;
}

// ============================================================================
// AuxiliaryGraph Implementation
// ============================================================================

AuxiliaryGraph::AuxiliaryGraph(const Graph& G_minus_P, const Path& P, int b, int L) {
    num_vertices = G_minus_P.get_vertex_count() + 1; // +1 for virtual root
    root_id = G_minus_P.get_vertex_count(); // Root is the new vertex

    // Copy edges from G_minus_P to weighted adjacency list (all weights = 1)
    const AdjacencyList& adj = G_minus_P.get_adj_list();
    for (const auto& [u, neighbors] : adj) {
        for (VertexID v : neighbors) {
            weighted_adj[u].push_back({v, 1});
        }
    }

    // Add edges from root to specific vertices on path P
    // Connect root to P[k] for k = b, b + 2L, b + 4L, ...
    // with weights i*L where i is the index in this sequence

    int path_len = P.size();
    for (int i = 0, k = b; k < path_len; i++, k += (2 * L)) {
        int weight = i * L;
        weighted_adj[root_id].push_back({P[k], weight});
    }
}

void AuxiliaryGraph::fill_RD_table(std::vector<std::vector<int>>& RD_table,
                                     const Path& P, int b, int L) {
    // Run Dijkstra from root
    SSSPResult sssp = Dijkstra(weighted_adj, root_id);

    int path_len = P.size();

    // Fill RD-table entries
    // For each anchor point k = b, b + 2L, b + 4L, ...
    for (int i = 0, k = b; k < path_len; i++, k += (2 * L)) {
        // For each jump j from 1 to L
        for (int j = 1; j <= L; j++) {
            int target_idx = k + j;

            // Check bounds
            if (target_idx >= path_len) {
                break;
            }

            VertexID target_vertex = P[target_idx];

            // Get distance from root to target
            int dist_from_root = INF;
            if (sssp.distances.find(target_vertex) != sssp.distances.end()) {
                dist_from_root = sssp.distances.at(target_vertex);
            }

            // The actual replacement distance from P[k] to P[k+j] is:
            // dist_from_root - (i * L)
            // But we need to check if it's within the threshold (i+1)*L

            if (dist_from_root != INF && dist_from_root <= (i + 1) * L) {
                int replacement_dist = dist_from_root - (i * L);
                RD_table[k][j] = std::min(RD_table[k][j], replacement_dist);
            }
        }
    }
}
