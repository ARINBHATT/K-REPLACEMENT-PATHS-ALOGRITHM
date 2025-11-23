#pragma once
#include "common/CommonDataStructures.h"
#include "common/CommonGraph.h"
#include <vector>
#include <cmath>

// Forward declaration of AuxiliaryGraph
class AuxiliaryGraph;

/**
 * ShortDetour module - handles short detours in replacement path algorithm
 * Computes RD-Table (Replacement Distance Table) for short detours
 * Using the approach from Roditty-Zwick algorithm
 */
class ShortDetour {
private:
    const Graph& original_graph;
    Graph graph_minus_path;
    Path shortest_path;
    int L; // threshold for short detours (typically sqrt(n))
    std::vector<std::vector<int>> RD_table;

public:
    /**
     * Constructor
     * @param G The original graph
     * @param P The shortest path from s to t
     */
    ShortDetour(const Graph& G, const Path& P);

    /**
     * Compute the RD-Table for all short detours
     * This fills the RD_table with replacement distances
     */
    void compute_RD_table();

    /**
     * Get the RD-table (after computation)
     */
    const std::vector<std::vector<int>>& get_RD_table() const { return RD_table; }

    /**
     * Get threshold L
     */
    int get_L() const { return L; }

    /**
     * Get the shortest path
     */
    const Path& get_path() const { return shortest_path; }

    /**
     * shortRepPath: Find replacement path when edge (i, i+1) on P is removed
     * This handles SHORT detours only (length <= 2L)
     * @param edge_index The index i where edge (P[i], P[i+1]) is removed
     * @return The replacement path, or empty vector if no short detour exists
     */
    Path shortRepPath(int edge_index);

    /**
     * Get replacement distance for a short detour from P[i] to P[i+j]
     * @param i Starting position on path
     * @param j Jump distance (1 <= j <= L)
     * @return Replacement distance, or INF if no such detour exists
     */
    int get_replacement_distance(int i, int j) const;

    friend class AuxiliaryGraph;
};

/**
 * AuxiliaryGraph - helper class for computing RD-Table entries
 * Creates an auxiliary graph with a virtual root node for efficient SSSP computation
 */
class AuxiliaryGraph {
private:
    WeightedAdjacencyList weighted_adj;
    int num_vertices;
    int root_id;

public:
    /**
     * Constructor
     * @param G_minus_P Graph with shortest path edges removed
     * @param P The shortest path
     * @param b Offset parameter (0 to 2L-1)
     * @param L Threshold value
     */
    AuxiliaryGraph(const Graph& G_minus_P, const Path& P, int b, int L);

    /**
     * Fill RD-Table entries using Dijkstra from virtual root
     * @param RD_table Reference to the RD table to fill
     * @param P The shortest path
     * @param b Offset parameter
     * @param L Threshold value
     */
    void fill_RD_table(std::vector<std::vector<int>>& RD_table, const Path& P, int b, int L);

    /**
     * Get the weighted adjacency list
     */
    const WeightedAdjacencyList& get_weighted_adj() const { return weighted_adj; }

    /**
     * Get the root ID
     */
    int get_root_id() const { return root_id; }
};
