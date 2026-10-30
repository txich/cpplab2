#include "recursive_graph_collections.h"

#include <limits.h>

#include <algorithm>
#include <cstddef>
#include <map>
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
using Collection = std::map<std::string, Graph>;
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

Graph *find_graph(long id, char const *name) {
	if (name == nullptr)
		return nullptr;

	auto *collection = find_collection(id);
	if (collection == nullptr)
		return nullptr;

	auto it = collection->find(name);
	return it == collection->end() ? nullptr : &it->second;
}

Neighbours *find_neighbours(long id, char const *name, int node) {
	auto *graph = find_graph(id, name);

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

// Compute the graph level excluding both stored directions of the selected
// edge.
ssize_t rgc_graph_level_without_edge(long id, char const *name, int w_1,
									 int w_2) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr) {
		ssize_t max_level = 0;

		for (auto node_it = graph->begin(); node_it != graph->end();
			 ++node_it) {
			for (auto neighbours_it = node_it->second.begin();
				 neighbours_it != node_it->second.end(); ++neighbours_it) {
				if (!((node_it->first == w_1 && neighbours_it->first == w_2) ||
					  (node_it->first == w_2 && neighbours_it->first == w_1))) {
					auto current_level = neighbours_it->second;
					max_level = std::max(max_level, current_level);
				}
			}
		}

		return max_level;
	}

	return -1;
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
			auto &graph = it->second;

			for (int i = 0; i < n; ++i) {
				graph.try_emplace(i);
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
	collection->erase(name);
}

int rgc_graph_new_node(long id, char const *name) {
	auto *graph = find_graph(id, name);

	if (graph != nullptr && graph->size() < static_cast<size_t>(INT_MAX)) {
		int new_node = 0;
		auto it = graph->begin();

		// In sorted order, the first gap is the smallest available vertex
		// number.
		while (it != graph->end() && it->first == new_node) {
			++new_node;
			++it;
		}

		graph->try_emplace(new_node);
		return new_node;
	}

	return -1;
}

bool rgc_graph_delete_node(long id, char const *name, int x) {
	auto *graph = find_graph(id, name);
	if (graph == nullptr)
		return false;
	auto node = graph->find(x);
	if (node == graph->end())
		return false;
	// Remove reverse entries before destroying this vertex's neighbour map.
	for (auto const &[neighbour, level] : node->second)
		graph->at(neighbour).erase(x);
	graph->erase(node);
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
				if (it_x->second.find(y) == it_x->second.end()) {
					it_x->second.try_emplace(y, START_LEVEL);
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

	auto iter = collection->find(name);
	if (iter == collection->end())
		return nullptr;

	iter++;
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
	auto *recipient_graph = find_graph(id, recipient_name);
	auto *donor_graph = find_graph(id, donor_name);

	if (recipient_graph != nullptr && donor_graph != nullptr &&
		is_name_correct(result_name)) {
		auto *w1_neighbours = find_neighbours(id, recipient_name, w_1);
		auto v1_it = donor_graph->find(v_1);
		auto v2_it = donor_graph->find(v_2);

		ssize_t nodes_recipient = static_cast<ssize_t>(recipient_graph->size());
		ssize_t nodes_donor = static_cast<ssize_t>(donor_graph->size());

		// The two selected donor vertices merge with existing recipient
		// vertices. Only the remaining donor vertices increase the result's
		// size.
		if (find_graph(id, result_name) == nullptr &&
			w1_neighbours != nullptr &&
			w1_neighbours->find(w_2) != w1_neighbours->end() &&
			v1_it != donor_graph->end() && v2_it != donor_graph->end() &&
			v_1 != v_2 && nodes_recipient <= INT_MAX - (nodes_donor - 2)) {
			auto edge_it = w1_neighbours->find(w_2);
			ssize_t replaced_edge_level = edge_it->second;
			ssize_t donor_level = rgc_graph_level(id, donor_name);
			ssize_t new_edge_level = 0;

			// An edgeless donor adds no levels and can replace a maximum-level
			// edge.
			if (rgc_count_edges(id, donor_name) > 0) {
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
					result_graph.try_emplace(next_free);
					++next_free;
				}
			}

			// All donor edges receive the new level, regardless of their old
			// levels.
			for (auto const &[donor_node, neighbours] : *donor_graph) {
				Node mapped_node = node_mapping.find(donor_node)->second;

				for (auto const &[donor_neighbour, level] : neighbours) {
					Node mapped_neighbour =
						node_mapping.find(donor_neighbour)->second;
					result_graph.find(mapped_node)
						->second.emplace(mapped_neighbour, new_edge_level);
				}
			}

			auto *collection = find_collection(id);
			// Insert the result only after validation and construction are
			// complete.
			auto [it, inserted] =
				collection->emplace(result_name, std::move(result_graph));

			return inserted;
		}
	}

	return false;
}

}  // namespace cxx
