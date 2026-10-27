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

Collection *find_collection(long id) {
	auto &all = collections();
	auto it = all.find(id);
	return it == all.end() ? nullptr : &it->second;
}

Graph *find_graph(long id, char const *name) {
	if (name == nullptr)
		return nullptr;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return nullptr;

	auto it = collection->find(name);
	return it == collection->end() ? nullptr : &it->second;
}
}  // namespace

namespace cxx {
using namespace std;

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
	auto *collection = find_collection(id);
	if (collection == nullptr)
		return -1;

	return static_cast<ssize_t>(collection->size());
}

bool rgc_new_graph(long id, char const *name, int n) {}	 // Oliwier

void rgc_delete_graph(long id, char const *name) {
	if (name == nullptr)
		return;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return;
	collection->erase(name);
}

int rgc_graph_new_node(long id, char const *name) {}  // Oliwier

bool rgc_graph_delete_node(long id, char const *name, int x) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return false;
	if (graph->erase(x) == 0)
		return false;
	for (auto &v : *graph)
		v.second.erase(x);
	return true;
}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {}	 // Oliwier

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return false;
	auto node_x = graph->find(x);
	auto node_y = graph->find(y);
	if (node_x == graph->end() || node_y == graph->end())
		return false;

	if (node_x->second.erase(y) == 0)
		return false;
	node_y->second.erase(x);
	return true;
}

char const *rgc_first_graph(long id) {}	 // Oliwier

char const *rgc_next_graph(long id, char const *name) {
	if (name == nullptr)
		return nullptr;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return nullptr;

	auto iter = collection->find(name);
	if (iter == collection->end())
		return nullptr;

	iter++;
	if (iter == collection->end())
		return nullptr;

	return iter->first.c_str();
}

int rgc_first_node(long id, char const *name) {}  // Oliwier

int rgc_next_node(long id, char const *name, int x) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return -1;

	auto iter = graph->upper_bound(x);
	if (iter == graph->end())
		return -1;

	return iter->first;
}

int rgc_first_edge(long id, char const *name, int x) {}	 // Oliwier

int rgc_next_edge(long id, char const *name, int x, int y) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return -1;

	auto iter = graph->find(x);
	if (iter == graph->end())
		return -1;

	auto node_y = iter->second.upper_bound(y);
	if (node_y == iter->second.end())
		return -1;

	return node_y->first;
}

bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2) {}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {}

ssize_t rgc_graph_level(long id, char const *name) {}

ssize_t rgc_max_level(void) {}

ssize_t rgc_count_nodes(long id, char const *name) {}

ssize_t rgc_count_edges(long id, char const *name) {}

}  // namespace cxx
