# Replacement Paths Algorithm

Implementation of the Roditty-Zwick Replacement Paths Algorithm for efficiently computing replacement paths when edges fail on a shortest path.

## Compilation

### Test Short Detours Only
```bash
g++ -std=c++17 -o test_short_detour \
    common/CommonGraph.cpp \
    common/CommonAlgorithms.cpp \
    ShortDetour.cpp \
    test_short_detour.cpp \
    -I. -Wall
```

### Test Complete Solution (Short + Long Detours)
```bash
g++ -std=c++17 -o test_integrated \
    common/CommonGraph.cpp \
    common/CommonAlgorithms.cpp \
    ShortDetour.cpp \
    ReplacementPaths.cpp \
    test_integrated.cpp \
    -I. -Wall
```

## Run

```bash
./test_short_detour
./test_integrated
```

## Structure

- `common/` - Shared data structures and algorithms (Graph, BFS, Dijkstra)
- `ShortDetour.h/.cpp` - Short detour module (handles detours ≤ 2L)
- `ReplacementPaths.h/.cpp` - Complete solution integrating short and long detours
- `test_*.cpp` - Test files demonstrating functionality
