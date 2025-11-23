# K-REPLACEMENT-PATHS-ALOGRITHM
# Replacement Paths Algorithm - Short Detour Integration

## Project Overview

This project implements the **Roditty-Zwick Replacement Paths Algorithm**, which efficiently computes replacement paths when edges fail on a shortest path. The implementation is divided into two main components:

1. **Short Detour Module** - Handles detours with length ≤ 2L (where L = ⌈√n⌉)
2. **Long Detour Module** - Handles detours with length > 2L

## What's Included

### Core Files

- **`common/`** - Shared data structures and algorithms
  - `CommonDataStructures.h` - Type definitions (VertexID, Path, AdjacencyList, etc.)
  - `CommonGraph.h/.cpp` - Graph class with path manipulation
  - `CommonAlgorithms.h/.cpp` - BFS, Dijkstra, path reconstruction

- **`ShortDetour.h/.cpp`** - Short detour implementation
  - RD-Table computation using auxiliary graphs
  - `shortRepPath()` function for finding short replacement paths
  - Integrated with common modules

### Test & Example Files

- **`test_short_detour.cpp`** - Comprehensive test suite
  - Tests the example graph from short_detour.cpp
  - Tests simple graphs with known detours
  - Verifies integration readiness

### 1. Compile and Run Tests

```bash
# Compile the test suite
g++ -std=c++17 -o test_short_detour \
    common/CommonGraph.cpp \
    common/CommonAlgorithms.cpp \
    ShortDetour.cpp \
    test_short_detour.cpp \
    -I. -Wall

# Run tests
./test_short_detour
```

