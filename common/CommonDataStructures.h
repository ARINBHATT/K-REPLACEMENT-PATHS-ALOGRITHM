#pragma once
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <limits>
#include <utility> // for std::pair

// A unique ID for each vertex (e.g., 0, 1, 2...)
using VertexID = int;

// The shortest path P, represented as an ordered list of vertices
// P = <u_0, u_1, ..., u_l>
using Path = std::vector<VertexID>;

// A standard adjacency list for an unweighted graph
// Maps a vertex to a list of its neighbors
using AdjacencyList = std::map<VertexID, std::vector<VertexID>>;

// A weighted adjacency list (for Person 1's auxiliary graph)
// Maps a vertex to a list of pairs: {neighbor, weight}
using WeightedAdjacencyList = std::map<VertexID, std::vector<std::pair<VertexID, int>>>;

// A standard "infinity" value
const int INF = std::numeric_limits<int>::max();

// Struct to return from SSSP (Shortest Path) functions
struct SSSPResult {
    std::map<VertexID, int> distances;
    std::map<VertexID, VertexID> parents;
};
