# Tests

Run from the project root:

```sh
python3 tests/run_tests.py
python3 tests/run_tests.py --sanitize
```

Requirements: Python 3, a C23 compiler, and a C++23 compiler and standard library
(including `<format>` for the supplied C++ example). The defaults are `gcc` and
`g++`; override them with `CC` and `CXX` if necessary. No Python packages are
required. Build outputs are placed in a temporary directory and removed on exit.

The runner tests `LEVEL_MAX=1,2,4,16`. Use `--levels 1 4` to select values for the
custom tests. The supplied examples always run with their expected levels, 4 and
16. `--sanitize` enables UndefinedBehaviorSanitizer for the module and executables.

Coverage includes:

- Every public API function, missing objects, null names, and invalid arguments.
- All nonzero single-byte names, empty/long names, and ownership of stored names.
- Collection isolation, identifier progression, and initialization before `main`.
- Ordered iteration, numbering gaps, duplicate edges, loops, and symmetric deletion.
- Counts and levels after mutations, including empty graphs and isolated vertices.
- Edge replacement, endpoint orientation, smallest available vertex numbers,
  self-replacement, edgeless donors, level limits, and unchanged input graphs.
- Reproducible randomized operations compared with an independent model storing
  a vertex set and one entry per undirected edge (seed `20261030`).
- C interoperability, the supplied examples, and both C++ link orders.
- Compile-time rejection of `LEVEL_MAX=0`.

The tests do not allocate `INT_MAX` vertices or exhaust all `long` identifiers.
They also do not decide the ambiguous `rgc_next_graph` behavior when a non-null
name is absent from a nonempty collection; iteration tests use existing names.
Compiler warnings remain visible and do not fail the run automatically.

## Large graphs

Run the stress suite separately:

```sh
python3 tests/run_tests.py --stress
python3 tests/run_tests.py --stress --sanitize --levels 4
python3 tests/run_tests.py --stress --edges 250000 --levels 4
```

Each base graph has exactly 100,000 edges by default (`--edges` changes this).
The suite covers a star, a path with gaps in vertex numbering, a dense bipartite
graph, and edge replacement using large donors. Self-replacement produces
199,999 edges and 200,000 vertices at the default size. It checks both directions
of edges, ordering, levels, counts, failed operations, deletion of high-degree
vertices, reuse of vertex numbers, and preservation of input graphs.

Each scenario prints elapsed time including graph creation, assertions, and
cleanup. These measurements have no pass/fail threshold: they depend on the
machine and do not predict the grading server's time limits. Large graphs are
created with `rgc_new_graph`; this suite does not benchmark building all vertices
one by one through `rgc_graph_new_node`.
