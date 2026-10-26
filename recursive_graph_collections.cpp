#include "recursive_graph_collections.h"

#include <limits.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#ifndef LEVEL_MAX
#define LEVEL_MAX 4
#endif

static_assert(LEVEL_MAX > 0, "LEVEL_MAX musi byc dodatnie");

namespace {
using Node = int;
using Level = ssize_t;
using Neighbours = std::map<Node, Level>;
using Graph = std::map<Node, Neighbours>;
using Collection = std::map<std::string, Graph>;
using Collections = std::unordered_map<long, Collection>;

Collections &collections() {
	static Collections data;
	return data;
}

// Tutaj należy umieścić prywatne typy i funkcje pomocnicze modułu.
// Kontenery kolekcji należy inicjować jako lokalne zmienne statyczne
// w funkcji pomocniczej, aby uniknąć problemu kolejności inicjalizacji.
}  // namespace

namespace cxx {
using namespace std;
// Global containers for collections.

long rgc_new_collection(void) {
	static long id = 0;
	if (id >= LONG_MAX)
		return -1;

	collections()[id] = Collection();
	return id++;
}

void rgc_delete_collection(long id) {
	collections().erase(id);
}

ssize_t rgc_size(long id) {
	auto &all = collections();
	auto it = all.find(id);

	if (it == all.end())
		return -1;

	return static_cast<ssize_t>(it->second.size());
}  // Anton

bool rgc_new_graph(long id, char const *name, int n) {}	 // Oliwer

void rgc_delete_graph(long id, char const *name) {}

int rgc_graph_new_node(long id, char const *name) {}

bool rgc_graph_delete_node(long id, char const *name, int x) {}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {}

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {}

char const *rgc_first_graph(long id) {}

char const *rgc_next_graph(long id, char const *name) {}

int rgc_first_node(long id, char const *name) {}

int rgc_next_node(long id, char const *name, int x) {}

int rgc_first_edge(long id, char const *name, int x) {}

int rgc_next_edge(long id, char const *name, int x, int y) {}

bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2) {}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {}

ssize_t rgc_graph_level(long id, char const *name) {}

ssize_t rgc_max_level(void) {}

ssize_t rgc_count_nodes(long id, char const *name) {}

ssize_t rgc_count_edges(long id, char const *name) {}

}  // namespace cxx
