#include "ShortDetour.h"
#include "common/CommonAlgorithms.h"
#include <iostream>
#include <algorithm>

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

    // Reserve space for auxiliary graphs
    auxiliary_graphs.resize(2 * L);

    // For each offset b from 0 to 2L-1
    for (int b = 0; b < 2 * L; b++) {
        // Create auxiliary graph and fill corresponding RD-table entries
        auto aux_graph = std::make_shared<AuxiliaryGraph>(graph_minus_path, shortest_path, b, L);
        aux_graph->fill_RD_table(RD_table, shortest_path, b, L);
        auxiliary_graphs[b] = aux_graph;
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
    int best_offset = -1; 
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
                // Determine which offset b was used for this entry
                best_offset = a % (2 * L);
            }
        }
    }

    // If no short detour found, return empty path
    if (best_start == -1 || best_offset == -1 || best_offset >= (int)auxiliary_graphs.size()) {
        return {};
    }

    Path replacement;

    // Add path prefix P[0..best_start]
    for (int i = 0; i <= best_start; i++) {
        replacement.push_back(shortest_path[i]);
    }

    // Reconstruct the actual detour path from P[best_start] to P[best_start + best_jump]
    // using parent pointers from the auxiliary graph
    VertexID detour_start = shortest_path[best_start];
    VertexID detour_end = shortest_path[best_start + best_jump];

    // Get the SSSP result from the appropriate auxiliary graph
    auto& aux_graph = auxiliary_graphs[best_offset];
    if (!aux_graph) {
        return {}; // Auxiliary graph not available
    }

    const SSSPResult& sssp = aux_graph->get_sssp_result();

    // Reconstruct path from detour_end back to the auxiliary root
    Path detour_reverse;
    VertexID curr = detour_end;

    if (sssp.parents.find(curr) == sssp.parents.end()) {
        return {}; // No path found in auxiliary graph
    }

    while (curr != aux_graph->get_root_id() && sssp.parents.find(curr) != sssp.parents.end()) {
        detour_reverse.push_back(curr);
        VertexID parent = sssp.parents.at(curr);
        if (parent == -1) break;
        curr = parent;
    }

    // Now we have the path from detour_end to some anchor point on P
    // We need to reverse it and skip the first element (detour_start) and last element (detour_end)
    std::reverse(detour_reverse.begin(), detour_reverse.end());

    // The detour path should go from detour_start to detour_end through G-P
    // Add the intermediate vertices (excluding detour_start which is already in replacement,
    // but including detour_end which will be the last vertex of the detour)
    for (size_t i = 0; i < detour_reverse.size(); i++) {
        VertexID v = detour_reverse[i];
        // Skip detour_start if it appears (already added)
        if (v == detour_start) continue;
        replacement.push_back(v);
    }

    // Add path suffix P[best_start+best_jump+1..end]
    // Note: detour_end = P[best_start+best_jump] is already added from detour reconstruction
    for (int i = best_start + best_jump + 1; i < path_len; i++) {
        replacement.push_back(shortest_path[i]);
    }

    return replacement;
}

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
    // Run Dijkstra from root and store result for path reconstruction
    sssp_result = Dijkstra(weighted_adj, root_id);

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
            if (sssp_result.distances.find(target_vertex) != sssp_result.distances.end()) {
                dist_from_root = sssp_result.distances.at(target_vertex);
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
