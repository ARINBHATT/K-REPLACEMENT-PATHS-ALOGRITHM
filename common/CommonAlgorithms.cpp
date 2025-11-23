#include "CommonAlgorithms.h"

SSSPResult BFS(const Graph& G, VertexID start_node) {
    SSSPResult res;
    const auto& adj = G.get_adj_list();
    
    // Initialize all known vertices to INF
    for (const auto& pair : adj) {
        res.distances[pair.first] = INF;
    }
    // Ensure start_node is in the map
    res.distances[start_node] = 0;
    res.parents[start_node] = -1; // -1 indicates root/no parent

    std::queue<VertexID> q;
    q.push(start_node);

    while (!q.empty()) {
        VertexID u = q.front();
        q.pop();

        // Safely access neighbors
        if (adj.find(u) != adj.end()) {
            for (VertexID v : adj.at(u)) {
                if (res.distances.find(v) == res.distances.end() || res.distances[v] == INF) {
                    res.distances[v] = res.distances[u] + 1;
                    res.parents[v] = u;
                    q.push(v);
                }
            }
        }
    }
    return res;
}

SSSPResult Dijkstra(const WeightedAdjacencyList& G_weighted, VertexID start_node) {
    SSSPResult res;
    
    // Initialize distances
    for (const auto& pair : G_weighted) {
        res.distances[pair.first] = INF;
    }
    res.distances[start_node] = 0;
    res.parents[start_node] = -1;

    // Min-priority queue: stores {distance, vertex}
    // Uses std::greater to make it a min-heap
    std::priority_queue<std::pair<int, VertexID>, 
                        std::vector<std::pair<int, VertexID>>, 
                        std::greater<std::pair<int, VertexID>>> pq;

    pq.push({0, start_node});

    while (!pq.empty()) {
        int d = pq.top().first;
        VertexID u = pq.top().second;
        pq.pop();

        // If we found a shorter way to u already, skip
        if (d > res.distances[u]) continue;

        if (G_weighted.find(u) != G_weighted.end()) {
            for (auto& edge : G_weighted.at(u)) {
                VertexID v = edge.first;
                int weight = edge.second;

                // Relaxation step
                if (res.distances.find(v) == res.distances.end() || res.distances[u] + weight < res.distances[v]) {
                    res.distances[v] = res.distances[u] + weight;
                    res.parents[v] = u;
                    pq.push({res.distances[v], v});
                }
            }
        }
    }
    return res;
}

Path reconstruct_path(const SSSPResult& result, VertexID s, VertexID t) {
    Path p;
    if (result.distances.find(t) == result.distances.end() || result.distances.at(t) == INF) {
        return p; // No path exists
    }

    VertexID curr = t;
    while (curr != -1) {
        p.push_back(curr);
        if (curr == s) break;
        
        if (result.parents.find(curr) == result.parents.end()) {
            // Should not happen if path exists, but safety check
            return {}; 
        }
        curr = result.parents.at(curr);
    }

    // Path is constructed t -> s, so we reverse it to get s -> t
    std::reverse(p.begin(), p.end());
    return p;
}
