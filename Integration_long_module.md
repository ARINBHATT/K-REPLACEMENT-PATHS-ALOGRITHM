### 1. **Long Detour Module** (Not yet integrated)

**Current status:**
- You have `long_detour.cpp` which implements the randomized algorithm
- It's a standalone program, not integrated with the common modules

**What needs to happen:**
- Extract the long detour logic into a reusable function
- Make it use the same `Graph`, `Path` types from common modules
- Interface should be something like:

```cpp
Path solve_long_detour(const Graph& G, const Path& P, int edge_index, int L) {
    // Use random sampling approach from long_detour.cpp
    // Find detours with length > 2L
    // Return the replacement path
}
```

### 2. **Complete Integration**

**Create a main program that combines both:**

```cpp
int main() {
    // 1. Build graph and find shortest path
    Graph g = build_or_load_graph();
    Path shortest_path = find_shortest_path(g, s, t);

    // 2. Initialize SHORT detour module
    ShortDetour sd(g, shortest_path);
    sd.compute_RD_table();
    int L = sd.get_L();

    // 3. For each edge, find replacement path
    for (int i = 0; i < shortest_path.size() - 1; i++) {

        // Try SHORT detour first (fast)
        Path replacement = sd.shortRepPath(i);

        if (replacement.empty()) {
            // No short detour, use LONG detour (slower)
            replacement = solve_long_detour(g, shortest_path, i, L);
        }

        // Now we have the best replacement path
        print_result(i, replacement);
    }
}
```

