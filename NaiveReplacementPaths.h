#pragma once

#include "common/CommonDataStructures.h"
#include "common/CommonGraph.h"
#include <vector>

/**
 * Naive/Brute Force Replacement Paths Algorithm
 * Time Complexity: O(mn) where m = edges, n = vertices
 *
 * For each edge on the shortest path:
 *   1. Remove that edge from the graph
 *   2. Run BFS from source to target
 *   3. Store the replacement path length
 *
 * This is the baseline algorithm mentioned in the Roditty-Zwick paper
 */
class NaiveReplacementPaths {
private:
    const Graph& graph;
    const Path& shortest_path;

    struct ReplacementResult {
        int length;
        Path path;

        ReplacementResult() : length(INF) {}
    };

    std::vector<ReplacementResult> replacement_results;

public:
    /**
     * Constructor
     * @param G The graph
     * @param P The shortest path from s to t
     */
    NaiveReplacementPaths(const Graph& G, const Path& P);

    /**
     * Compute all replacement paths using naive approach
     * For each edge: remove it, run BFS, restore it
     */
    void compute_all_replacement_paths();

    /**
     * Get replacement for a specific edge
     */
    const ReplacementResult& get_replacement(int edge_index) const;

    /**
     * Get all results
     */
    const std::vector<ReplacementResult>& get_all_replacements() const {
        return replacement_results;
    }

    /**
     * Print summary
     */
    void print_summary() const;
};
