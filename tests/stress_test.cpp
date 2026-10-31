#include "recursive_graph_collections.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <chrono>
#include <climits>
#include <iomanip>
#include <iostream>
#include <string>

using namespace cxx;

void make_star(long id, char const *name, int edges) {
    assert(rgc_new_graph(id, name, edges + 1));
    for (int leaf = 1; leaf <= edges; ++leaf)
        assert(rgc_graph_add_edge(id, name, 0, leaf));
    assert(rgc_count_nodes(id, name) == edges + 1);
    assert(rgc_count_edges(id, name) == edges);
    assert(rgc_graph_level(id, name) == 0);
}

void test_star(int edges) {
    const long id = rgc_new_collection();
    make_star(id, "star", edges);
    int expected = 1;
    for (int leaf = rgc_first_edge(id, "star", 0); leaf != -1;
         leaf = rgc_next_edge(id, "star", 0, leaf)) {
        assert(leaf == expected++);
        assert(rgc_first_edge(id, "star", leaf) == 0);
        assert(rgc_next_edge(id, "star", leaf, 0) == -1);
        assert(rgc_edge_level(id, "star", leaf, 0) == 0);
    }
    assert(expected == edges + 1);
    for (int leaf = 3; leaf <= edges; leaf += 3) {
        assert(!rgc_graph_add_edge(id, "star", leaf, 0));
        assert(rgc_graph_delete_edge(id, "star", leaf, 0));
        assert(!rgc_graph_delete_edge(id, "star", 0, leaf));
        assert(rgc_first_edge(id, "star", leaf) == -1);
    }
    assert(rgc_count_edges(id, "star") == edges - edges / 3);
    assert(rgc_count_nodes(id, "star") == edges + 1);
    for (int leaf = 3; leaf <= edges; leaf += 3)
        assert(rgc_graph_add_edge(id, "star", 0, leaf));
    assert(rgc_count_edges(id, "star") == edges);

    // Remove a vertex incident to every edge, and verify all reverse entries.
    assert(rgc_graph_delete_node(id, "star", 0));
    assert(rgc_count_nodes(id, "star") == edges);
    assert(rgc_count_edges(id, "star") == 0);
    for (int leaf = 1; leaf <= edges; ++leaf)
        assert(rgc_first_edge(id, "star", leaf) == -1);
    assert(rgc_graph_new_node(id, "star") == 0);
    // Exercise finding a free number beyond a long contiguous prefix.
    for (int i = 1; i <= 16; ++i)
        assert(rgc_graph_new_node(id, "star") == edges + i);
    rgc_delete_collection(id);
}

void test_sparse_path(int edges) {
    const long id = rgc_new_collection();
    assert(rgc_new_graph(id, "path", 2 * edges + 1));
    for (int node = 1; node < 2 * edges; node += 2)
        assert(rgc_graph_delete_node(id, "path", node));
    for (int i = 0; i < edges; ++i)
        assert(rgc_graph_add_edge(id, "path", 2 * i, 2 * i + 2));
    assert(rgc_count_nodes(id, "path") == edges + 1);
    assert(rgc_count_edges(id, "path") == edges);
    int expected = 0;
    for (int node = rgc_first_node(id, "path"); node != -1;
         node = rgc_next_node(id, "path", node)) {
        assert(node == expected);
        expected += 2;
        const int first = node == 0 ? 2 : node - 2;
        assert(rgc_first_edge(id, "path", node) == first);
        assert(rgc_next_edge(id, "path", node, node) ==
               (node == 2 * edges ? -1 : node + 2));
    }
    assert(expected == 2 * edges + 2);
    for (int i = 0; i < edges; i += 2)
        assert(rgc_graph_delete_edge(id, "path", 2 * i, 2 * i + 2));
    assert(rgc_count_edges(id, "path") == edges / 2);
    for (int i = 0; i < edges; ++i) {
        const int level = i % 2 == 0 ? -1 : 0;
        assert(rgc_edge_level(id, "path", 2 * i, 2 * i + 2) == level);
        assert(rgc_edge_level(id, "path", 2 * i + 2, 2 * i) == level);
    }
    for (int i = 0; i < 16 && i < edges; ++i)
        assert(rgc_graph_new_node(id, "path") == 2 * i + 1);
    rgc_delete_collection(id);
}

void test_dense_bipartite(int edges) {
    int left = 1;
    while (1LL * left * left < edges)
        ++left;
    const int right = (edges + left - 1) / left;
    const long id = rgc_new_collection();
    assert(rgc_new_graph(id, "dense", left + right));
    for (int i = 0; i < edges; ++i)
        assert(rgc_graph_add_edge(id, "dense", i / right, left + i % right));
    assert(rgc_count_edges(id, "dense") == edges);
    long long visits = 0;
    for (int x = 0; x < left + right; ++x) {
        int previous = -1;
        for (int y = rgc_first_edge(id, "dense", x); y != -1;
             y = rgc_next_edge(id, "dense", x, y)) {
            assert(y > previous);
            assert((x < left) != (y < left));
            const int a = x < left ? x : y;
            const int b = x < left ? y - left : x - left;
            assert(a * right + b < edges);
            assert(rgc_edge_level(id, "dense", y, x) == 0);
            previous = y;
            ++visits;
        }
    }
    assert(visits == 2LL * edges);
    for (int x = 0; x < left; ++x)
        assert(rgc_graph_delete_node(id, "dense", x));
    assert(rgc_count_nodes(id, "dense") == right);
    assert(rgc_count_edges(id, "dense") == 0);
    for (int y = left; y < left + right; ++y)
        assert(rgc_first_edge(id, "dense", y) == -1);
    rgc_delete_collection(id);
}

void test_large_replacement(int edges) {
    const long id = rgc_new_collection();
    make_star(id, "donor", edges);
    assert(rgc_new_graph(id, "recipient", 3));
    assert(rgc_graph_add_edge(id, "recipient", 0, 1));
    assert(rgc_graph_add_edge(id, "recipient", 1, 2));
    const bool allowed = LEVEL_MAX > 1;
    assert(rgc_replace_edge(id, "result", "recipient", 0, 1,
                            "donor", 1, 2) == allowed);
    if (allowed) {
        assert(rgc_count_nodes(id, "result") == edges + 2);
        assert(rgc_count_edges(id, "result") == edges + 1);
        assert(rgc_graph_level(id, "result") == 1);
        assert(rgc_edge_level(id, "result", 1, 2) == 0);
        for (int leaf = 1; leaf <= edges; ++leaf) {
            const int mapped_leaf = leaf <= 2 ? leaf - 1 : leaf + 1;
            assert(rgc_edge_level(id, "result", 3, mapped_leaf) == 1);
            assert(rgc_edge_level(id, "result", mapped_leaf, 3) == 1);
        }
        assert(rgc_graph_delete_node(id, "result", 3));
        assert(rgc_count_edges(id, "result") == 1);
        assert(rgc_graph_level(id, "result") == 0);
        rgc_delete_graph(id, "result");

        assert(rgc_replace_edge(id, "self", "donor", 0, 1, "donor", 0, 1));
        assert(rgc_count_nodes(id, "self") == 2 * edges);
        assert(rgc_count_edges(id, "self") == 2 * edges - 1);
        int expected = 1;
        for (int leaf = rgc_first_edge(id, "self", 0); leaf != -1;
             leaf = rgc_next_edge(id, "self", 0, leaf)) {
            assert(leaf == expected++);
            const int level = leaf == 1 || leaf > edges ? 1 : 0;
            assert(rgc_edge_level(id, "self", leaf, 0) == level);
        }
        assert(expected == 2 * edges);
        if (LEVEL_MAX == 2) {
            const auto size_before = rgc_size(id);
            assert(!rgc_replace_edge(id, "overflow", "self", 0, 1,
                                     "donor", 0, 1));
            assert(rgc_size(id) == size_before);
        }
        assert(rgc_graph_delete_node(id, "self", 0));
        assert(rgc_count_edges(id, "self") == 0);
        rgc_delete_graph(id, "self");
    } else {
        assert(rgc_count_nodes(id, "result") == -1);
        assert(rgc_size(id) == 2);
    }

    // Large edgeless donors must also work when LEVEL_MAX is 1.
    assert(rgc_new_graph(id, "empty", edges + 1));
    assert(rgc_replace_edge(id, "no_new_edges", "donor", 0, 1, "empty", 0, 1));
    assert(rgc_count_nodes(id, "no_new_edges") == 2 * edges);
    assert(rgc_count_edges(id, "no_new_edges") == edges - 1);
    assert(rgc_graph_level(id, "no_new_edges") == 0);
    assert(rgc_edge_level(id, "no_new_edges", 0, 1) == -1);
    for (int leaf = 2; leaf <= edges; ++leaf)
        assert(rgc_edge_level(id, "no_new_edges", 0, leaf) == 0);
    for (int node = edges + 1; node < 2 * edges; ++node)
        assert(rgc_first_edge(id, "no_new_edges", node) == -1);

    // Replacement must leave both input graphs unchanged.
    assert(rgc_count_nodes(id, "recipient") == 3);
    assert(rgc_count_edges(id, "recipient") == 2);
    assert(rgc_edge_level(id, "recipient", 0, 1) == 0);
    assert(rgc_edge_level(id, "recipient", 1, 2) == 0);
    assert(rgc_count_nodes(id, "donor") == edges + 1);
    assert(rgc_count_edges(id, "donor") == edges);
    for (int leaf = 1; leaf <= edges; ++leaf) {
        assert(rgc_edge_level(id, "donor", 0, leaf) == 0);
        assert(rgc_edge_level(id, "donor", leaf, 0) == 0);
    }
    rgc_delete_collection(id);
}

void test_incremental_vertices(int nodes) {
    const long id = rgc_new_collection();
    assert(rgc_new_graph(id, "incremental", 0));
    for (int node = 0; node < nodes; ++node)
        assert(rgc_graph_new_node(id, "incremental") == node);
    for (int node = 0; node < nodes; node += 2)
        assert(rgc_graph_delete_node(id, "incremental", node));
    for (int node = 0; node < nodes; node += 2)
        assert(rgc_graph_new_node(id, "incremental") == node);
    assert(rgc_graph_new_node(id, "incremental") == nodes);
    assert(rgc_count_nodes(id, "incremental") == nodes + 1);
    assert(rgc_count_edges(id, "incremental") == 0);
    rgc_delete_collection(id);
}

void run_case(char const *name, void (*test)(int), int edges) {
    const auto start = std::chrono::steady_clock::now();
    test(edges);
    const auto seconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start).count();
    std::cout << "PASS: " << name << ", size=" << edges
              << ", " << std::fixed << std::setprecision(3) << seconds
              << " s (including assertions)" << std::endl;
}

int main(int argc, char **argv) {
    const int edges = argc == 2 ? std::stoi(argv[1]) : 100000;
    assert(edges >= 2 && edges <= (INT_MAX - 2) / 2);
    assert(rgc_max_level() == LEVEL_MAX - 1);
    run_case("incremental vertices and number reuse", test_incremental_vertices, edges);
    run_case("star: high-degree deletion and edge updates", test_star, edges);
    run_case("path: gaps and ordered iteration", test_sparse_path, edges);
    run_case("dense bipartite graph", test_dense_bipartite, edges);
    run_case("large donors and self-replacement", test_large_replacement, edges);
    std::cout << "All stress tests passed, LEVEL_MAX=" << LEVEL_MAX << '\n';
}
