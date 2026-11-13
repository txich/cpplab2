# Recursive Graph Collections

A C++23 library for managing collections of named, undirected simple graphs
through a C-compatible API. Graphs can be built recursively by replacing an edge
with another graph, while tracking the level of each inserted edge.

## Features

- Independent collections with unique, increasing identifiers.
- Named graphs with vertex and edge insertion, deletion, and ordered iteration.
- Recursive edge replacement, including using a graph as its own donor.
- Vertex and edge counts, edge levels, and a configurable maximum level.
- A shared header for C and C++; C++ callers use the `cxx` namespace.

## How edge replacement works

`rgc_replace_edge` creates a new graph from a recipient and a donor in the same
collection:

1. Copy the recipient and remove its selected edge `{w_1, w_2}`.
2. Map the selected donor vertices `v_1` and `v_2` to `w_1` and `w_2`, respectively.
3. Visit the remaining donor vertices in ascending order, assigning each the
   smallest unused nonnegative vertex number in the result.
4. Copy all donor edges through that mapping. Each receives the level
   `max(donor graph level, replaced edge level) + 1`.

The input graphs remain unchanged, and retained recipient edges keep their
levels. The result must have a new, valid name. A graph's level is its greatest
edge level, or `0` if it has no edges. An edgeless donor adds no new edge levels
and can replace an edge even at the maximum level.

`LEVEL_MAX` defaults to `4`, allowing levels `0` through `3`. Set it with
`-DLEVEL_MAX=N` when compiling **the implementation**; it must be positive.
`rgc_max_level()` reports the compiled limit minus one. Replacement fails if the
result would exceed the level limit or contain more than `INT_MAX` vertices.

## Build

Requirements:

- A C++23 compiler and standard library.
- A C23 compiler to build the C usage example below.
- An environment providing POSIX `ssize_t` through `<sys/types.h>`.

Run the commands below from the repository root. The library uses the C++
standard library and has no third-party dependencies.

Compile the implementation into an object file:

```sh
mkdir -p build
g++ -std=c++23 -Wall -Wextra -O2 -c recursive_graph_collections.cpp -o build/recursive_graph_collections.o
```

To change the level limit, add a flag such as `-DLEVEL_MAX=16` to this command.

## Usage

This C example replaces a single edge with a three-vertex path:

```c
#include "recursive_graph_collections.h"
#include <assert.h>

int main(void) {
    long id = rgc_new_collection();
    assert(id >= 0);

    assert(rgc_new_graph(id, "Edge", 2));
    assert(rgc_graph_add_edge(id, "Edge", 0, 1));

    assert(rgc_new_graph(id, "Path", 3));
    assert(rgc_graph_add_edge(id, "Path", 0, 1));
    assert(rgc_graph_add_edge(id, "Path", 1, 2));

    assert(rgc_replace_edge(id, "Expanded", "Edge", 0, 1, "Path", 0, 2));
    assert(rgc_count_nodes(id, "Expanded") == 3);
    assert(rgc_count_edges(id, "Expanded") == 2);
    assert(rgc_graph_level(id, "Expanded") == 1);
    assert(rgc_count_edges(id, "Edge") == 1);

    rgc_delete_collection(id);
    return 0;
}
```

Save it as `main.c` and, after building the library above, run:

```sh
gcc -std=c23 -Wall -Wextra -O2 -I. -c main.c -o build/main.o
g++ build/main.o build/recursive_graph_collections.o -o build/main
./build/main
```

Compile C callers with the C compiler and link with the C++ compiler to include
the C++ runtime. Keep assertions enabled for this example; it exits silently on
success and requires `LEVEL_MAX` of at least `2`.

In C++, include the same header and qualify API calls with `cxx::`, for example
`cxx::rgc_new_collection()`. The header provides C linkage for both languages.

## API conventions

The complete interface is declared in
[recursive_graph_collections.h](recursive_graph_collections.h).

- Collection IDs start at `0`, increase with each creation, and are never reused.
  Creation returns `-1` after the `long` identifier range is exhausted.
- Graph names are nonempty strings matching `[A-Za-z0-9_]+`, unique within a
  collection. The library stores its own copies.
- New graphs have vertices numbered `0` through `n - 1` and no edges. New
  vertices reuse the smallest available number; deletion leaves other vertex
  numbers unchanged and removes all incident edges.
- Edges are undirected, with no loops or duplicates. Manually added edges start
  at level `0`.
- Graph iteration follows ASCII lexicographic order; vertex and neighbour
  iteration follows numeric order. The `rgc_next_*` functions return the first
  value strictly greater than the supplied threshold, even if that threshold
  is absent. Neighbour iteration still requires the source vertex to exist.
- Graph-name pointers returned by iteration belong to the library and remain
  valid until that graph or its collection is deleted. Do not free them.
- Boolean operations return `false` on failure. Numeric queries and vertex
  iteration return `-1` for missing results or invalid arguments; graph-name
  iteration returns a null pointer. Deleting a missing graph or collection does
  nothing.

## Tests

Run the test suite from the repository root:

```sh
python3 tests/run_tests.py
```

Tests require Python 3, a C23 compiler, and a C++23 compiler and standard library
with `<format>` support for the C++ example. No additional Python packages are
needed. The default compilers are `gcc` and `g++`; override them with `CC` and
`CXX` if needed.

The suite checks the public API and randomized operations against an independent
model at `LEVEL_MAX=1,2,4,16`. It also checks C interoperability, initialization
before `main` in both C++ link orders, and rejection of `LEVEL_MAX=0`.

To enable UndefinedBehaviorSanitizer or run the separate large-graph suite:

```sh
python3 tests/run_tests.py --sanitize
python3 tests/run_tests.py --stress --levels 4
```

Build outputs are created in temporary directories and removed on exit. See
[tests/README.md](tests/README.md) for coverage details and additional options.

## Implementation and files

The implementation uses STL containers: an `unordered_map` for collections,
ordered maps for graph names and adjacency lists, and a set of reusable vertex
numbers. Each undirected edge is stored at both endpoints; separate statistics
track total edges and counts by level. Storage is initialized on first use to
support calls before `main`, and internal helpers are hidden in an anonymous
namespace.

- [recursive_graph_collections.h](recursive_graph_collections.h) — public C/C++ API.
- [recursive_graph_collections.cpp](recursive_graph_collections.cpp) — implementation.
- [tests/](tests/) — test runner, functional and stress tests, reference model,
  and C/C++ example programs.
