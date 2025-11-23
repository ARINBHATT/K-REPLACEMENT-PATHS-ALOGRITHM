#include "CommonGraph.h"

Graph::Graph(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    int u, v;
    int num_edges;
    // Reading num_vertices and num_edges (standard competitive programming format)
    infile >> vertex_count >> num_edges;

    // Initialize empty adjacency lists for all vertices to ensure they exist in map
    for(int i=0; i < vertex_count; ++i) {
        adj_list[i] = {};
    }

    for (int i = 0; i < num_edges; ++i) {
        infile >> u >> v;
        add_edge(u, v);
    }
}

void Graph::add_edge(VertexID u, VertexID v) {
    adj_list[u].push_back(v);
    // Ensure v exists in the map even if it has no outgoing edges
    if (adj_list.find(v) == adj_list.end()) {
        adj_list[v] = {};
    }
    // If strictly adding new nodes, update count. 
    // (Assuming nodes are 0 to N-1 based on file format, but this is safer)
    if (adj_list.size() > (size_t)vertex_count) {
        vertex_count = adj_list.size();
    }
}

void Graph::remove_edge(VertexID u, VertexID v) {
    if (adj_list.find(u) != adj_list.end()) {
        auto& neighbors = adj_list[u];
        // Remove all instances of v from u's list
        neighbors.erase(std::remove(neighbors.begin(), neighbors.end(), v), neighbors.end());
    }
}

Graph Graph::create_graph_minus_path(const Path& P) const {
    Graph temp = *this; // Create a copy
    if (P.size() < 2) return temp;

    for (size_t i = 0; i < P.size() - 1; ++i) {
        VertexID u = P[i];
        VertexID v = P[i+1];
        temp.remove_edge(u, v); // Remove the directed edge on the path
    }
    return temp;
}

Graph Graph::create_reversed_graph() const {
    Graph reversed;
    reversed.vertex_count = this->vertex_count;

    // Initialize nodes
    for (auto const& [u, neighbors] : adj_list) {
        if (reversed.adj_list.find(u) == reversed.adj_list.end()) {
             reversed.adj_list[u] = {};
        }
        for (VertexID v : neighbors) {
            reversed.add_edge(v, u); // Swap direction
        }
    }
    return reversed;
}
