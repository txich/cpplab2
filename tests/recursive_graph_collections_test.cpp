#include "recursive_graph_collections.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <climits>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace cxx;
using Snapshot = std::map<int, std::map<int, ssize_t>>;

// Exercise the public API during initialization, before main().
const long startup_id = rgc_new_collection();

Snapshot snapshot(long id, char const *name) {
    Snapshot result;
    const auto node_count = rgc_count_nodes(id, name);
    assert(node_count >= 0);
    for (int x = rgc_first_node(id, name); x != -1;
         x = rgc_next_node(id, name, x)) {
        assert(result.try_emplace(x).second);
        assert(static_cast<ssize_t>(result.size()) <= node_count);
        for (int y = rgc_first_edge(id, name, x); y != -1;
             y = rgc_next_edge(id, name, x, y)) {
            const auto level = rgc_edge_level(id, name, x, y);
            assert(level >= 0 && level <= rgc_max_level());
            assert(result.at(x).emplace(y, level).second);
            assert(result.at(x).size() < static_cast<size_t>(node_count));
        }
    }
    assert(static_cast<ssize_t>(result.size()) == node_count);
    return result;
}

void check_missing_graph(long id, char const *name) {
    assert(rgc_graph_new_node(id, name) == -1);
    assert(!rgc_graph_delete_node(id, name, 0));
    assert(!rgc_graph_add_edge(id, name, 0, 1));
    assert(!rgc_graph_delete_edge(id, name, 0, 1));
    assert(rgc_first_node(id, name) == -1);
    assert(rgc_next_node(id, name, 0) == -1);
    assert(rgc_first_edge(id, name, 0) == -1);
    assert(rgc_next_edge(id, name, 0, 1) == -1);
    assert(rgc_edge_level(id, name, 0, 1) == -1);
    assert(rgc_graph_level(id, name) == -1);
    assert(rgc_count_nodes(id, name) == -1);
    assert(rgc_count_edges(id, name) == -1);
    rgc_delete_graph(id, name);
}

void test_collections_and_names() {
    assert(startup_id == 0);
    assert(rgc_size(startup_id) == 0);
    assert(rgc_first_graph(startup_id) == nullptr);
    assert(rgc_next_graph(startup_id, "missing") == nullptr);
    const long id = rgc_new_collection();
    assert(id == 1);
    rgc_delete_collection(startup_id);
    const long next_id = rgc_new_collection();
    assert(next_id == 2); // Deleted identifiers must not be reused.
    for (long missing : {-1L, LONG_MAX, startup_id}) {
        assert(rgc_size(missing) == -1);
        assert(rgc_first_graph(missing) == nullptr);
        assert(rgc_next_graph(missing, "G") == nullptr);
        assert(!rgc_new_graph(missing, "G", 1));
        check_missing_graph(missing, "G");
        rgc_delete_collection(missing);
    }
    check_missing_graph(id, nullptr);
    check_missing_graph(id, "missing");
    assert(!rgc_new_graph(id, nullptr, 0));
    assert(!rgc_new_graph(id, "", 0));
    assert(!rgc_new_graph(id, "negative", -1));
    // Check every possible nonzero byte against the specified ASCII alphabet.
    for (unsigned value = 1; value <= UCHAR_MAX; ++value) {
        char name[] = {static_cast<char>(value), '\0'};
        const bool valid = (value >= 'a' && value <= 'z') ||
                           (value >= 'A' && value <= 'Z') ||
                           (value >= '0' && value <= '9') || value == '_';
        assert(rgc_new_graph(id, name, 0) == valid);
        rgc_delete_graph(id, name);
    }
    for (const char *bad : {"bad name", "a-b", "a.b", "a/b", "a\nb"})
        assert(!rgc_new_graph(id, bad, 0));
    std::string long_name(4096, 'a');
    assert(rgc_new_graph(id, long_name.c_str(), 0));
    rgc_delete_graph(id, long_name.c_str());
    char name[] = "Owned";
    assert(rgc_new_graph(id, name, 0));
    name[0] = 'X';
    assert(rgc_count_nodes(id, "Owned") == 0);
    const char *stored_name = rgc_first_graph(id);
    assert(stored_name && std::strcmp(stored_name, "Owned") == 0);
    const std::vector<std::string> expected = {"0", "A", "AA", "Owned", "_", "a", "z"};
    for (const char *value : {"z", "a", "_", "AA", "A", "0"})
        assert(rgc_new_graph(id, value, 0));
    assert(std::strcmp(stored_name, "Owned") == 0);
    assert(!rgc_new_graph(id, "Owned", 5));
    assert(rgc_count_nodes(id, "Owned") == 0);
    std::vector<std::string> names;
    for (const char *current = rgc_first_graph(id); current;
         current = rgc_next_graph(id, current)) {
        names.emplace_back(current);
        assert(names.size() <= expected.size());
    }
    assert(names == expected);
    assert(rgc_next_graph(id, nullptr) == nullptr);
    assert(rgc_size(next_id) == 0); // Collections are independent.
    rgc_delete_collection(id);
    rgc_delete_collection(next_id);
}

void test_nodes_and_edges() {
    const long id = rgc_new_collection();
    assert(rgc_new_graph(id, "G", 5));
    assert(rgc_count_edges(id, "G") == 0);
    assert(rgc_graph_level(id, "G") == 0);
    assert(rgc_first_node(id, "G") == 0);
    assert(rgc_first_edge(id, "G", 0) == -1);
    assert(rgc_graph_add_edge(id, "G", 0, 2));
    assert(rgc_graph_add_edge(id, "G", 2, 4));
    assert(!rgc_graph_add_edge(id, "G", 2, 0));
    assert(!rgc_graph_add_edge(id, "G", 2, 2));
    assert(!rgc_graph_add_edge(id, "G", -1, 2));
    assert(!rgc_graph_add_edge(id, "G", 2, INT_MAX));
    assert(rgc_edge_level(id, "G", 2, 0) == 0);
    assert(rgc_first_edge(id, "G", 2) == 0);
    assert(rgc_next_edge(id, "G", 2, 1) == 4);
    assert(rgc_next_edge(id, "G", 2, INT_MAX) == -1);
    assert(rgc_graph_delete_node(id, "G", 1));
    assert(rgc_graph_delete_node(id, "G", 3));
    assert(rgc_next_node(id, "G", 1) == 2);
    assert(rgc_next_node(id, "G", INT_MIN) == 0);
    assert(rgc_next_node(id, "G", INT_MAX) == -1);
    assert(rgc_graph_new_node(id, "G") == 1);
    assert(rgc_graph_new_node(id, "G") == 3);
    assert(rgc_graph_new_node(id, "G") == 5);
    assert(!rgc_graph_delete_node(id, "G", 99));
    assert(rgc_graph_delete_edge(id, "G", 2, 0));
    assert(rgc_edge_level(id, "G", 0, 2) == -1);
    assert(!rgc_graph_delete_edge(id, "G", 0, 2));
    assert(rgc_count_nodes(id, "G") == 6);
    assert(rgc_count_edges(id, "G") == 1);
    assert(rgc_graph_delete_node(id, "G", 2));
    assert(rgc_first_edge(id, "G", 4) == -1);
    assert(rgc_count_edges(id, "G") == 0);
    for (int node : {0, 1, 3, 4, 5})
        assert(rgc_graph_delete_node(id, "G", node));
    assert(rgc_first_node(id, "G") == -1);
    assert(rgc_graph_level(id, "G") == 0);
    assert(rgc_graph_new_node(id, "G") == 0);
    rgc_delete_collection(id);
}

void test_replacement() {
    const long id = rgc_new_collection();
    assert(rgc_new_graph(id, "R", 5));
    assert(rgc_graph_delete_node(id, "R", 1));
    assert(rgc_graph_delete_node(id, "R", 3));
    assert(rgc_graph_add_edge(id, "R", 0, 4));
    assert(rgc_graph_add_edge(id, "R", 0, 2));
    assert(rgc_new_graph(id, "D", 6));
    assert(rgc_graph_delete_node(id, "D", 2));
    for (auto [x, y] : {std::pair{4, 0}, {0, 1}, {1, 3}, {3, 5}})
        assert(rgc_graph_add_edge(id, "D", x, y));
    const auto recipient = snapshot(id, "R");
    const auto donor = snapshot(id, "D");
    const bool allowed = LEVEL_MAX > 1;
    assert(rgc_replace_edge(id, "result", "R", 4, 0, "D", 4, 5) == allowed);
    assert(snapshot(id, "R") == recipient);
    assert(snapshot(id, "D") == donor);
    if (allowed) {
        const Snapshot expected = {
            {0, {{2, 0}, {5, 1}}}, {1, {{3, 1}, {4, 1}}},
            {2, {{0, 0}}}, {3, {{1, 1}, {5, 1}}},
            {4, {{1, 1}}}, {5, {{0, 1}, {3, 1}}}
        };
        assert(snapshot(id, "result") == expected);
        assert(rgc_count_edges(id, "result") == 5);
        assert(rgc_graph_level(id, "result") == 1);
        assert(rgc_graph_delete_node(id, "result", 1));
        assert(rgc_graph_delete_node(id, "result", 5));
        assert(rgc_graph_level(id, "result") == 0);
    } else {
        assert(rgc_count_nodes(id, "result") == -1);
    }
    const auto size_before = rgc_size(id);
    assert(!rgc_replace_edge(id, nullptr, "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "", "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad name", "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "R", "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "D", "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad", nullptr, 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad", "R", 0, 4, nullptr, 0, 1));
    assert(!rgc_replace_edge(id, "bad", "missing", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad", "R", 0, 4, "missing", 0, 1));
    assert(!rgc_replace_edge(-1, "bad", "R", 0, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad", "R", 2, 4, "D", 0, 1));
    assert(!rgc_replace_edge(id, "bad", "R", 0, 4, "D", 0, 0));
    assert(!rgc_replace_edge(id, "bad", "R", 0, 4, "D", 0, 2));
    assert(rgc_size(id) == size_before);
    assert(snapshot(id, "R") == recipient);
    assert(snapshot(id, "D") == donor);

    // An edgeless donor creates no new edge levels, even at LEVEL_MAX == 1.
    assert(rgc_new_graph(id, "empty_donor", 5));
    assert(rgc_replace_edge(id, "no_new_edges", "R", 0, 4, "empty_donor", 4, 0));
    assert(rgc_count_nodes(id, "no_new_edges") == 6);
    assert(rgc_count_edges(id, "no_new_edges") == 1);
    assert(rgc_graph_level(id, "no_new_edges") == 0);
    rgc_delete_collection(id);
}

void test_levels_and_self_replacement() {
    const long id = rgc_new_collection();
    assert(rgc_max_level() == LEVEL_MAX - 1);
    assert(rgc_new_graph(id, "base", 3));
    assert(rgc_graph_add_edge(id, "base", 0, 1));
    assert(rgc_new_graph(id, "empty", 2));
    std::string current = "base";
    for (int level = 1; level < LEVEL_MAX; ++level) {
        const std::string next = "L" + std::to_string(level);
        assert(rgc_replace_edge(id, next.c_str(), current.c_str(), 0, 1,
                                "base", 0, 1));
        assert(rgc_edge_level(id, next.c_str(), 0, 1) == level);
        assert(rgc_graph_level(id, next.c_str()) == level);
        current = next;
    }
    const auto original = snapshot(id, current.c_str());
    const auto before_size = rgc_size(id);
    assert(!rgc_replace_edge(id, "overflow", current.c_str(), 0, 1, "base", 0, 1));
    assert(!rgc_replace_edge(id, "overflow2", "base", 0, 1, current.c_str(), 0, 1));
    assert(rgc_size(id) == before_size);
    assert(snapshot(id, current.c_str()) == original);
    assert(rgc_replace_edge(id, "removed", current.c_str(), 0, 1, "empty", 0, 1));
    assert(rgc_graph_level(id, "removed") == 0);
    assert(rgc_count_edges(id, "removed") == 0);
    if (LEVEL_MAX > 1) {
        assert(rgc_replace_edge(id, "self", "base", 0, 1, "base", 1, 0));
        assert(rgc_count_nodes(id, "self") == 4);
        assert(rgc_edge_level(id, "self", 0, 1) == 1);
        assert(rgc_edge_level(id, "base", 0, 1) == 0);
        const char *maximum = LEVEL_MAX == 2 ? "self" : current.c_str();
        assert(rgc_graph_add_edge(id, maximum, 0, 2));
        assert(rgc_replace_edge(id, "keep_max", maximum, 0, 2, "empty", 0, 1));
        assert(rgc_graph_level(id, "keep_max") == LEVEL_MAX - 1);
    }
    rgc_delete_collection(id);
}

int main() {
    test_collections_and_names();
    test_nodes_and_edges();
    test_replacement();
    test_levels_and_self_replacement();
    std::cout << "PASS: deterministic API tests, LEVEL_MAX=" << LEVEL_MAX << '\n';
}
