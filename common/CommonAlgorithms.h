#pragma once
#include "CommonDataStructures.h"
#include "CommonGraph.h"

// BFS for unweighted graphs
SSSPResult BFS(const Graph& G, VertexID start_node);

// Dijkstra for weighted graphs (Note: takes WeightedAdjacencyList, not Graph class)
SSSPResult Dijkstra(const WeightedAdjacencyList& G_weighted, VertexID start_node);

// Helper function to reconstruct the initial path P from SSSP result
Path reconstruct_path(const SSSPResult& result, VertexID s, VertexID t);
