#pragma once

#include "common/CommonDataStructures.h"
#include "common/CommonGraph.h"
#include "ShortDetour.h"
#include <vector>
#include <map>
#include <set>

/**
 * Complete Replacement Paths Solution
 * Integrates Short Detour (RD-Table) and Long Detour (Sampling) algorithms
 * Based on Roditty-Zwick paper
 */
class ReplacementPaths {
private:
    const Graph& graph;
    const Path& shortest_path;
    int L;

    ShortDetour short_detour_module;

    // Results storage
    struct ReplacementResult {
        Path path;
        int length;
        bool is_short_detour;

        ReplacementResult() : length(INF), is_short_detour(false) {}
    };

    std::vector<ReplacementResult> replacement_results;

    /**
     * Solve long detours using sampling algorithm (Section 2.2 of paper)
     * @return Map from edge_index to replacement path length
     */
    std::map<int, int> solve_long_detours();

    /**
     * BFS from a node, excluding edges on the shortest path
     * @param start Starting vertex
     * @param path_edges Set of edges to exclude
     * @param use_reverse Use reverse graph edges
     * @return Distance map
     */
    std::vector<int> bfs_excluding_path(
        VertexID start,
        const std::set<std::pair<VertexID, VertexID>>& path_edges,
        bool use_reverse) const;

public:
    /**
     * Constructor
     * @param G The graph
     * @param P The shortest path from s to t
     */
    ReplacementPaths(const Graph& G, const Path& P);

    /**
     * Compute all replacement paths
     * For each edge on P, find the best replacement path when that edge fails
     */
    void compute_all_replacement_paths();

    /**
     * Get replacement path for a specific edge removal
     * @param edge_index Index of edge to remove (0 to |P|-2)
     * @return Replacement path information
     */
    const ReplacementResult& get_replacement(int edge_index) const;

    /**
     * Get all replacement results
     */
    const std::vector<ReplacementResult>& get_all_replacements() const {
        return replacement_results;
    }

    /**
     * Get threshold L
     */
    int get_L() const { return L; }

    /**
     * Print summary of all replacements
     */
    void print_summary() const;
};
