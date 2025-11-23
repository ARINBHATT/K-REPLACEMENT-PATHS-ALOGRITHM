#pragma once
#include "CommonDataStructures.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

class Graph {
private:
    AdjacencyList adj_list;
    int vertex_count = 0;

public:
    // Default constructor
    Graph() = default;

    // Constructor: Loads from a file
    // File Format Assumption: 
    // Line 1: <num_vertices> <num_edges>
    // Subsequent lines: <source> <destination>
    Graph(const std::string& filename);

    // Add an edge (u, v)
    void add_edge(VertexID u, VertexID v);

    // Remove an edge (u, v)
    void remove_edge(VertexID u, VertexID v);

    // Get a copy of the graph with all edges in P removed
    Graph create_graph_minus_path(const Path& P) const;

    // Get a reversed (transposed) copy of the graph
    Graph create_reversed_graph() const;

    // Getters
    const AdjacencyList& get_adj_list() const { return adj_list; }
    int get_vertex_count() const { return vertex_count; }
};
