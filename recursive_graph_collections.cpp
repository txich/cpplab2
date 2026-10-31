#include "recursive_graph_collections.h"

#include <limits.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
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
// The first element stores the graph, and the second stores its numbering data.
using GraphData = std::pair<Graph, NodeNumbers>;
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

Collection *find_collection(long id) {
	auto &all = collections();
	auto it = all.find(id);
	return it == all.end() ? nullptr : &it->second;
}

GraphData *find_graph_data(Collection *collection, char const *name) {
	if (collection == nullptr || name == nullptr)
		return nullptr;

	auto it = collection->find(name);
	return it == collection->end() ? nullptr : &it->second;
}

Graph *find_graph(long id, char const *name) {
	auto *data = find_graph_data(find_collection(id), name);
	return data == nullptr ? nullptr : &data->first;
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

}  // namespace

namespace cxx {
using namespace std;

long rgc_new_collection(void) {
	static long next_id = 0;
	if (next_id == -1)
		return -1;

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
	if (collection == nullptr)
		return -1;

	return static_cast<ssize_t>(collection->size());
}

bool rgc_new_graph(long id, char const *name, int n) {
	auto *collection = find_collection(id);

	if (collection != nullptr && is_name_correct(name) && n >= 0) {
		auto [it, inserted] = collection->try_emplace(name);
		if (inserted) {
			auto &[graph, numbering] = it->second;
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
	if (name == nullptr)
		return;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return;
	auto it = collection->find(name);
	if (it != collection->end())
		collection->erase(it);
}

int rgc_graph_new_node(long id, char const *name) {
	auto *data = find_graph_data(find_collection(id), name);
	if (data == nullptr)
		return -1;
	auto &[graph, numbering] = *data;

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
	if (data == nullptr)
		return false;
	auto &[graph, numbering] = *data;
	auto node = graph.find(x);
	if (node == graph.end())
		return false;
	// Remove reverse entries before destroying this vertex's neighbour map.
	for (auto const &[neighbour, level] : node->second)
		graph.at(neighbour).erase(x);
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
	auto *graph = find_graph(id, name);

	if (graph != nullptr) {
		auto it_x = graph->find(x);
		if (it_x != graph->end()) {
			auto it_y = graph->find(y);
			if (it_y != graph->end() && it_x != it_y) {
				// Checking one endpoint is sufficient because adjacency is
				// symmetric.
				auto [edge, inserted] =
					it_x->second.try_emplace(y, START_LEVEL);
				if (inserted) {
					it_y->second.try_emplace(x, START_LEVEL);

					return true;
				}
			}
		}
	}

	return false;
}

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

char const *rgc_first_graph(long id) {
	auto *collection = find_collection(id);

	if (collection != nullptr && !collection->empty()) {
		return collection->begin()->first.c_str();
	}

	return nullptr;
}

char const *rgc_next_graph(long id, char const *name) {
	if (name == nullptr)
		return nullptr;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return nullptr;

	// The threshold name need not belong to an existing graph.
	auto iter = collection->upper_bound(name);
	if (iter == collection->end())
		return nullptr;

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
	if (graph == nullptr)
		return -1;

	// The threshold x need not be an existing vertex number.
	auto iter = graph->upper_bound(x);
	if (iter == graph->end())
		return -1;

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
	if (graph == nullptr)
		return -1;

	auto iter = graph->find(x);
	if (iter == graph->end())
		return -1;

	// The threshold y need not be an existing neighbour.
	auto node_y = iter->second.upper_bound(y);
	if (node_y == iter->second.end())
		return -1;

	return node_y->first;
}

ssize_t rgc_edge_level(long id, char const *name, int x, int y) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return -1;
	auto node_x = graph->find(x);
	if (node_x == graph->end())
		return -1;
	auto node_y = node_x->second.find(y);
	if (node_y == node_x->second.end())
		return -1;
	return node_y->second;
}

ssize_t rgc_graph_level(long id, char const *name) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr) {
		ssize_t max_level = 0;

		for (auto node_it = graph->begin(); node_it != graph->end();
			 ++node_it) {
			for (auto neighbours_it = node_it->second.begin();
				 neighbours_it != node_it->second.end(); ++neighbours_it) {
				auto current_level = neighbours_it->second;
				max_level = max(max_level, current_level);
			}
		}

		return max_level;
	}

	return -1;
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
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return -1;

	size_t count = 0;
	for (auto const &[node, neighbours] : *graph)
		count += neighbours.size();

	// Each undirected edge contributes twice to the sum of vertex degrees.
	return static_cast<ssize_t>(count / 2);
}

bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2) {
	auto *collection = find_collection(id);
	auto *recipient_data = find_graph_data(collection, recipient_name);
	auto *donor_data = find_graph_data(collection, donor_name);
	auto *recipient_graph =
		recipient_data == nullptr ? nullptr : &recipient_data->first;
	auto *donor_graph = donor_data == nullptr ? nullptr : &donor_data->first;

	if (recipient_graph != nullptr && donor_graph != nullptr &&
		is_name_correct(result_name)) {
		auto result_position = collection->lower_bound(result_name);
		if (result_position != collection->end() &&
			result_position->first == result_name)
			return false;

		auto *w1_neighbours = find_neighbours(recipient_graph, w_1);
		if (w1_neighbours == nullptr)
			return false;
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
			ssize_t donor_level = 0;
			bool donor_has_edges = false;
			// Determine the donor's level and edge presence in one traversal.
			for (auto const &[node, neighbours] : *donor_graph) {
				donor_has_edges = donor_has_edges || !neighbours.empty();
				for (auto const &[neighbour, level] : neighbours)
					donor_level = max(donor_level, level);
			}
			ssize_t new_edge_level = 0;

			// An edgeless donor adds no levels and can replace a maximum-level
			// edge.
			if (donor_has_edges) {
				// Check the level constraint for the resulting graph.
				if (max(donor_level, replaced_edge_level) < LEVEL_MAX - 1) {
					new_edge_level = max(donor_level, replaced_edge_level) + 1;
				} else {
					return false;
				}
			}

			// A separate result preserves both inputs, even if they are the
			// same graph.
			Graph result_graph = *recipient_graph;
			// The result owns an independent copy of the vertex-number
			// allocator.
			NodeNumbers result_numbers = recipient_data->second;
			auto &[next_unused, free_nodes] = result_numbers;

			// Remove the edge w_1 - w_2 from the resulting graph.
			result_graph.find(w_1)->second.erase(w_2);
			result_graph.find(w_2)->second.erase(w_1);

			// The map associates each vertex of the donor graph
			// with its corresponding vertex in the resulting graph.
			unordered_map<Node, Node> node_mapping;
			node_mapping.reserve(donor_graph->size());

			node_mapping.emplace(v_1, w_1);
			node_mapping.emplace(v_2, w_2);

			Node next_free = 0;
			// Retain the scan position to avoid rescanning occupied numbers.
			auto occupied_it = result_graph.begin();
			// The ordered map assigns new numbers in ascending donor-vertex
			// order.
			for (auto const &[donor_node, neighbours] : *donor_graph) {
				if (donor_node != v_1 && donor_node != v_2) {
					// Find the smallest unused vertex number.
					while (occupied_it != result_graph.end() &&
						   occupied_it->first == next_free) {
						++next_free;
						++occupied_it;
					}

					node_mapping.emplace(donor_node, next_free);
					result_graph.try_emplace(occupied_it, next_free);
					// Update the numbering data by filling a gap or extending
					// the range.
					if (next_free < next_unused)
						free_nodes.erase(next_free);
					else
						next_unused = next_free + 1;
					++next_free;
				}
			}

			// All donor edges receive the new level, regardless of their old
			// levels.
			for (auto const &[donor_node, neighbours] : *donor_graph) {
				if (neighbours.empty())
					continue;
				Node mapped_node = node_mapping.find(donor_node)->second;
				auto &mapped_neighbours =
					result_graph.find(mapped_node)->second;

				for (auto const &[donor_neighbour, level] : neighbours) {
					Node mapped_neighbour =
						node_mapping.find(donor_neighbour)->second;
					mapped_neighbours.emplace(mapped_neighbour, new_edge_level);
				}
			}

			// Reuse the insertion position after validation and construction.
			collection->emplace_hint(
				result_position, result_name,
				GraphData{std::move(result_graph), std::move(result_numbers)});

			return true;
		}
	}

	return false;
}

}  // namespace cxx
