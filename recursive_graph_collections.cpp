#include "recursive_graph_collections.h"

#include <limits.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>

#ifndef LEVEL_MAX
#define LEVEL_MAX 4
#endif

static_assert(LEVEL_MAX > 0, "LEVEL_MAX musi być dodatnie.");

namespace {
using Node = int;
using Level = ssize_t;
// Each undirected edge is stored at both endpoints with the same level.
using Neighbours = std::map<Node, Level>;
// An empty neighbour map represents an isolated vertex.
using Graph = std::map<Node, Neighbours>;
// The first element is one past the largest vertex number, or zero if empty.
// The second element stores all free numbers below that boundary.
using NodeNumbers = std::pair<Node, std::set<Node>>;
// The map associates each vertex of the donor graph
// with its corresponding vertex in the resulting graph.
using NodeMapping = std::unordered_map<Node, Node>;
// Only levels with a positive number of undirected edges are stored.
using LevelCounts = std::map<Level, std::size_t>;
// Total edge count and counts by level; each undirected edge is counted once.
using EdgeStats = std::pair<std::size_t, LevelCounts>;
using GraphData = std::tuple<Graph, NodeNumbers, EdgeStats>;
// The transparent comparator std::less<> compares stored strings directly with
// const char* lookup keys, avoiding temporary strings while preserving lexical
// order.
using Collection = std::map<std::string, GraphData, std::less<>>;
using Collections = std::unordered_map<long, Collection>;

constexpr Level START_LEVEL = 0;

Collections &collections() {
	// Initialization on first use avoids dependencies on other global objects.
	static Collections data;
	return data;
}

// GCC reserve priorities 0--100 for the runtime. Initialize before
// ordinary global objects so the state outlives their destructors.
constexpr int INITIALIZATION_PRIORITY = 101;

[[gnu::constructor(INITIALIZATION_PRIORITY)]] void initialize_collections() {
	(void)collections();
}

Collection *find_collection(long id) {
	auto &all = collections();
	auto it = all.find(id);
	return it == all.end() ? nullptr : &it->second;
}

GraphData *find_graph_data(Collection *collection, char const *name) {
	if (collection == nullptr || name == nullptr) {
		return nullptr;
	}
	auto it = collection->find(name);
	return it == collection->end() ? nullptr : &it->second;
}

Graph *find_graph(long id, char const *name) {
	auto *data = find_graph_data(find_collection(id), name);
	return data == nullptr ? nullptr : &std::get<Graph>(*data);
}

void add_edge_counts(EdgeStats &stats, Level level, std::size_t count = 1) {
	if (count == 0) {
		return;
	}
	auto &[total, counts] = stats;
	auto entry = counts.try_emplace(level, 0).first;
	entry->second += count;
	total += count;
}

// Called only when an edge of this level is present.
void remove_edge_count(EdgeStats &stats, Level level) {
	auto &[total, counts] = stats;
	auto entry = counts.find(level);
	if (--entry->second == 0) {
		counts.erase(entry);
	}
	--total;
}

Level max_edge_level(EdgeStats const &stats) {
	auto const &counts = stats.second;
	return counts.empty() ? START_LEVEL : counts.rbegin()->first;
}

Neighbours *find_neighbours(Graph *graph, int node) {
	if (graph != nullptr) {
		auto it_node = graph->find(node);

		if (it_node != graph->end()) {
			return &it_node->second;
		}
	}

	return nullptr;
}

bool is_name_correct(char const *name) {
	if (name != nullptr) {
		size_t i = 0;
		char character = name[i];

		// Explicit ranges enforce the required alphabet independently of
		// locale.
		while (character != '\0') {
			if (!((character >= 'a' && character <= 'z') ||
				  (character >= 'A' && character <= 'Z') ||
				  (character >= '0' && character <= '9') || character == '_')) {
				return false;
			}
			++i;
			character = name[i];
		}

		if (i == 0) {  // If i is 0, the string is empty.
			return false;
		}
	} else {
		return false;
	}

	return true;
}

// Adds donor nodes to the result graph and builds their mapping.
void add_donor_nodes(Graph const &donor_graph, Graph &result_graph,
					 NodeNumbers &numbers, NodeMapping &node_mapping, Node v_1,
					 Node v_2) {
	auto &[next_after_max, free_nodes] = numbers;

	Node next_free = 0;
	auto occupied_it = result_graph.begin();

	for (auto const &donor_vertex : donor_graph) {
		Node donor_node = donor_vertex.first;

		// The two selected donor vertices already have mappings.
		if (donor_node != v_1 && donor_node != v_2) {
			// Find the smallest unused vertex number.
			while (occupied_it != result_graph.end() &&
				   occupied_it->first == next_free) {
				++occupied_it;
				++next_free;
			}

			node_mapping.emplace(donor_node, next_free);
			result_graph.try_emplace(occupied_it, next_free);

			// Update the vertex numbering data.
			if (next_free < next_after_max) {
				free_nodes.erase(next_free);
			} else {
				next_after_max = next_free + 1;
			}

			++next_free;
		}
	}
}

// Adds donor edges to the result graph using the vertex mapping.
void add_donor_edges(Graph const &donor_graph, Graph &result_graph,
					 NodeMapping const &node_mapping, Level new_level) {
	for (auto const &[donor_node, neighbours] : donor_graph) {
		auto &mapped_neighbours =
			result_graph.find(node_mapping.at(donor_node))->second;

		for (auto const &edge : neighbours) {
			Node mapped_neighbour = node_mapping.at(edge.first);
			mapped_neighbours.try_emplace(mapped_neighbour, new_level);
		}
	}
}

// Copies the recipient, including numbering and edge statistics, then replaces
// the selected edge with the donor graph.
GraphData build_result_graph(GraphData const &recipient_graph_data,
							 GraphData const &donor_graph_data, Node w_1,
							 Node w_2, Node v_1, Node v_2,
							 Level new_edge_level) {
	GraphData result_graph_data = recipient_graph_data;
	auto &[result_graph, result_graph_numbers, result_stats] =
		result_graph_data;
	auto const &donor_graph = std::get<Graph>(donor_graph_data);

	// Remove both adjacency entries, but count the removed edge only once.
	auto &w1_neighbours = result_graph.find(w_1)->second;
	auto replaced_edge = w1_neighbours.find(w_2);
	remove_edge_count(result_stats, replaced_edge->second);
	w1_neighbours.erase(replaced_edge);
	result_graph.find(w_2)->second.erase(w_1);

	// The map associates each node of the donor graph
	// with its corresponding node in the result graph.
	NodeMapping node_mapping;
	node_mapping.reserve(donor_graph.size());

	node_mapping.emplace(v_1, w_1);
	node_mapping.emplace(v_2, w_2);

	// Add the remaining donor nodes and build mapping.
	add_donor_nodes(donor_graph, result_graph, result_graph_numbers,
					node_mapping, v_1, v_2);

	// Add donor edges with their new level.
	add_donor_edges(donor_graph, result_graph, node_mapping, new_edge_level);
	add_edge_counts(result_stats, new_edge_level,
					std::get<EdgeStats>(donor_graph_data).first);

	return result_graph_data;
}

}  // namespace

namespace cxx {
using namespace std;

long rgc_new_collection(void) {
	static long next_id = 0;
	if (next_id == -1) {
		return -1;
	}
	const long id = next_id;
	collections().try_emplace(id);
	// The -1 sentinel marks exhaustion without incrementing LONG_MAX.
	next_id = id == LONG_MAX ? -1 : id + 1;
	return id;
}

void rgc_delete_collection(long id) {
	collections().erase(id);
}

ssize_t rgc_size(long id) {
	auto *collection = find_collection(id);
	if (collection == nullptr) {
		return -1;
	}
	return static_cast<ssize_t>(collection->size());
}

bool rgc_new_graph(long id, char const *name, int n) {
	auto *collection = find_collection(id);

	if (collection != nullptr && is_name_correct(name) && n >= 0) {
		auto [it, inserted] = collection->try_emplace(name);
		if (inserted) {
			auto &[graph, numbering, stats] = it->second;
			auto &[next_unused, free_nodes] = numbering;
			next_unused = n;

			for (int i = 0; i < n; ++i) {
				graph.try_emplace(graph.end(), i);
			}

			return true;
		}
	}

	return false;
}

void rgc_delete_graph(long id, char const *name) {
	if (name == nullptr) {
		return;
	}
	auto *collection = find_collection(id);
	if (collection == nullptr) {
		return;
	}
	auto it = collection->find(name);
	if (it != collection->end()) {
		collection->erase(it);
	}
}

int rgc_graph_new_node(long id, char const *name) {
	auto *data = find_graph_data(find_collection(id), name);
	if (data == nullptr) {
		return -1;
	}
	auto &[graph, numbering, stats] = *data;

	if (graph.size() < static_cast<size_t>(INT_MAX)) {
		auto &[next_unused, free_nodes] = numbering;
		Node new_node;
		// Fill the smallest gap before extending the vertex-number range.
		if (free_nodes.empty()) {
			new_node = next_unused++;
		} else {
			auto first_free = free_nodes.begin();
			new_node = *first_free;
			free_nodes.erase(first_free);
		}

		graph.try_emplace(new_node);
		return new_node;
	}

	return -1;
}

bool rgc_graph_delete_node(long id, char const *name, int x) {
	auto *data = find_graph_data(find_collection(id), name);
	if (data == nullptr) {
		return false;
	}
	auto &[graph, numbering, stats] = *data;
	auto node = graph.find(x);
	if (node == graph.end()) {
		return false;
	}
	// Remove reverse entries before destroying this vertex's neighbour map.
	for (auto const &[neighbour, level] : node->second) {
		graph.at(neighbour).erase(x);
		remove_edge_count(stats, level);
	}
	graph.erase(node);
	auto &[next_unused, free_nodes] = numbering;
	if (x == next_unused - 1) {
		// Numbers beyond the last vertex no longer need separate free entries.
		next_unused = graph.empty() ? 0 : graph.rbegin()->first + 1;
		free_nodes.erase(free_nodes.lower_bound(next_unused), free_nodes.end());
	} else {
		free_nodes.insert(x);
	}
	return true;
}

bool rgc_graph_add_edge(long id, char const *name, int x, int y) {
	auto *data = find_graph_data(find_collection(id), name);

	if (data != nullptr) {
		auto &[graph, numbering, stats] = *data;
		auto it_x = graph.find(x);
		if (it_x != graph.end()) {
			auto it_y = graph.find(y);
			if (it_y != graph.end() && it_x != it_y) {
				// Checking one endpoint is sufficient because adjacency is
				// symmetric.
				auto [edge, inserted] =
					it_x->second.try_emplace(y, START_LEVEL);
				if (inserted) {
					it_y->second.try_emplace(x, START_LEVEL);
					add_edge_counts(stats, START_LEVEL);

					return true;
				}
			}
		}
	}

	return false;
}

bool rgc_graph_delete_edge(long id, char const *name, int x, int y) {
	auto *data = find_graph_data(find_collection(id), name);
	if (data == nullptr) {
		return false;
	}
	auto &[graph, numbering, stats] = *data;
	auto node_x = graph.find(x);
	auto node_y = graph.find(y);
	if (node_x == graph.end() || node_y == graph.end()) {
		return false;
	}
	auto edge = node_x->second.find(y);
	if (edge == node_x->second.end()) {
		return false;
	}
	remove_edge_count(stats, edge->second);
	node_x->second.erase(edge);
	node_y->second.erase(x);
	return true;
}

char const *rgc_first_graph(long id) {
	auto *collection = find_collection(id);

	if (collection != nullptr && !collection->empty()) {
		return collection->begin()->first.c_str();
	}

	return nullptr;
}

char const *rgc_next_graph(long id, char const *name) {
	if (name == nullptr) {
		return nullptr;
	}

	auto *collection = find_collection(id);
	if (collection == nullptr) {
		return nullptr;
	}

	// The threshold name need not belong to an existing graph.
	auto iter = collection->upper_bound(name);
	if (iter == collection->end()) {
		return nullptr;
	}

	return iter->first.c_str();
}

int rgc_first_node(long id, char const *name) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr && !graph->empty()) {
		return graph->begin()->first;
	}

	return -1;
}

int rgc_next_node(long id, char const *name, int x) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr) {
		return -1;
	}

	// The threshold x need not be an existing vertex number.
	auto iter = graph->upper_bound(x);
	if (iter == graph->end()) {
		return -1;
	}

	return iter->first;
}

int rgc_first_edge(long id, char const *name, int x) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr) {
		auto it = graph->find(x);
		if (it != graph->end() && !it->second.empty()) {
			return it->second.begin()->first;
		}
	}

	return -1;
}

int rgc_next_edge(long id, char const *name, int x, int y) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr) {
		return -1;
	}

	auto iter = graph->find(x);
	if (iter == graph->end()) {
		return -1;
	}

	// The threshold y need not be an existing neighbour.
	auto node_y = iter->second.upper_bound(y);
	if (node_y == iter->second.end()) {
		return -1;
	}

	return node_y->first;
}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr) {
		return -1;
	}
	auto node_x = graph->find(x);
	if (node_x == graph->end()) {
		return -1;
	}
	auto node_y = node_x->second.find(y);
	if (node_y == node_x->second.end()) {
		return -1;
	}
	return node_y->second;
}

ssize_t rgc_graph_level(long id, char const *name) {
	auto const *data = find_graph_data(find_collection(id), name);
	return data == nullptr ? -1 : max_edge_level(std::get<EdgeStats>(*data));
}

ssize_t rgc_max_level(void) {
	return LEVEL_MAX - 1;
}

ssize_t rgc_count_nodes(long id, char const *name) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr) {
		return static_cast<ssize_t>(graph->size());
	}

	return -1;
}

ssize_t rgc_count_edges(long id, char const *name) {
	auto const *data = find_graph_data(find_collection(id), name);
	if (data == nullptr) {
		return -1;
	}
	return static_cast<ssize_t>(std::get<EdgeStats>(*data).first);
}

bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2) {
	auto *collection = find_collection(id);
	auto *recipient_data = find_graph_data(collection, recipient_name);
	auto *donor_data = find_graph_data(collection, donor_name);
	auto *recipient_graph =
		recipient_data == nullptr ? nullptr : &std::get<Graph>(*recipient_data);
	auto *donor_graph =
		donor_data == nullptr ? nullptr : &std::get<Graph>(*donor_data);

	if (recipient_graph != nullptr && donor_graph != nullptr &&
		is_name_correct(result_name)) {
		auto result_position = collection->lower_bound(result_name);
		if (result_position != collection->end() &&
			result_position->first == result_name) {
			return false;
		}

		auto *w1_neighbours = find_neighbours(recipient_graph, w_1);
		if (w1_neighbours == nullptr) {
			return false;
		}
		auto edge_it = w1_neighbours->find(w_2);
		auto v1_it = donor_graph->find(v_1);
		auto v2_it = donor_graph->find(v_2);

		ssize_t nodes_recipient = static_cast<ssize_t>(recipient_graph->size());
		ssize_t nodes_donor = static_cast<ssize_t>(donor_graph->size());

		// The two selected donor vertices merge with existing recipient
		// vertices. Only the remaining donor vertices increase the result's
		// size.
		if (edge_it != w1_neighbours->end() && v1_it != donor_graph->end() &&
			v2_it != donor_graph->end() && v_1 != v_2 &&
			nodes_recipient <= INT_MAX - (nodes_donor - 2)) {
			ssize_t replaced_edge_level = edge_it->second;
			auto const &donor_stats = std::get<EdgeStats>(*donor_data);
			ssize_t donor_level = max_edge_level(donor_stats);
			ssize_t new_edge_level = 0;

			// An edgeless donor adds no levels and can replace a maximum-level
			// edge.
			if (donor_stats.first != 0) {
				// Check the level constraint for the resulting graph.
				if (max(donor_level, replaced_edge_level) < LEVEL_MAX - 1) {
					new_edge_level = max(donor_level, replaced_edge_level) + 1;
				} else {
					return false;
				}
			}

			// Build the result graph after successful validation.
			GraphData result_data =
				build_result_graph(*recipient_data, *donor_data, w_1, w_2, v_1,
								   v_2, new_edge_level);

			// Insert the completed graph into the collection
			collection->emplace_hint(result_position, result_name,
									 std::move(result_data));

			return true;
		}
	}

	return false;
}

}  // namespace cxx
