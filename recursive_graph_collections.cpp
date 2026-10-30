#include "recursive_graph_collections.h"

#include <limits.h>

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <cstddef>

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

	constexpr Level START_LEVEL = 0;

	Collections &collections() {
		static Collections data;
		return data;
	}

	// FUNKCJE POMOCNICZE
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
			char znak = name[i];

			while (znak != '\0') {
				if (!((znak >= 'a' && znak <= 'z') ||
					(znak >= 'A' && znak <= 'Z') ||
					(znak >= '0' && znak <= '9') ||
					znak == '_')) {
					return false;
				}
				++i;
				znak = name[i];
			}

			if (i == 0) { // Jeżeli i jest równe 0, to słowo jest puste.
				return false;
			}
		}
		else {
			return false;
		}

		return true;
	}

	// Oblicza poziom grafu bez wybranej krawędzi łączącej wierzchołki w_1 i w_2.
	// mozna poprawic aby w parametrach byly: referencja do konkretnego grafu, w1, w2.
	ssize_t rgc_graph_level_without_edge(long id, char const *name,
										int w_1, int w_2) {
		auto *graph = find_graph(id, name);

		if (graph != nullptr) {
			ssize_t max_level = 0;

			for (auto node_it = graph->begin(); node_it != graph->end();
				++node_it) {

				for (auto neighbours_it = node_it->second.begin();
					neighbours_it != node_it->second.end(); ++neighbours_it) {

					if (!((node_it->first == w_1 && neighbours_it->first == w_2)
						|| (node_it->first == w_2 && neighbours_it->first == w_1))) {
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
	} // Oliwier

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

			while (it != graph->end() && it->first == new_node) {
				++new_node;
				++it;
			}

			graph->try_emplace(new_node);
			return new_node;
		}

		return -1;
	}  // Oliwier

	bool rgc_graph_delete_node(long id, char const *name, int x) {
		auto *graph = find_graph(id, name);
		if (graph == nullptr)
			return false;
		auto node = graph->find(x);
		if (node == graph->end())
			return false;
		for (auto const &[neighbour, level] : node->second)
			graph->at(neighbour).erase(x);
		graph->erase(node);
		return true;
	}

	// Tutaj mozna wykorzystac funckje pomocnicza find_neighbours.
	bool rgc_graph_add_edge(long id, char const *name, int x, int y) {
		auto *graph = find_graph(id, name);

		if (graph != nullptr) {
			auto it_x = graph->find(x);
			if (it_x != graph->end()) {
				auto it_y = graph->find(y);
				if (it_y != graph->end() && it_x != it_y) {

					// Wystarczy, że jeden z węzłów x, y
					// nie ma drugiego wśród sąsiadów,
					// ponieważ graf jest nieskierowany.
					if (!it_x->second.contains(y)) {
						it_x->second.try_emplace(y, START_LEVEL);
						it_y->second.try_emplace(x, START_LEVEL);

						return true;
					}
				}
			}
		}

		return false;
	}	 // Oliwier

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
	}	 // Oliwier

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
	}  // Oliwier

	int rgc_next_node(long id, char const *name, int x) {
		auto *graph = find_graph(id, name);
		if (graph == nullptr)
			return -1;

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
	}	 // Oliwier

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
	}  // Oliwier

	ssize_t rgc_max_level(void) {
		return LEVEL_MAX - 1;
	}

	ssize_t rgc_count_nodes(long id, char const *name) {
		auto *graph = find_graph(id, name);

		if (graph != nullptr) {
			return static_cast<ssize_t>(graph->size());
		}

		return -1;
	}  // Oliwier

	ssize_t rgc_count_edges(long id, char const *name) {
		auto *graph = find_graph(id, name);
		if (graph == nullptr)
			return -1;

		size_t count = 0;
		for (auto const &[node, neighbours] : *graph)
			count += neighbours.size();

		return static_cast<ssize_t>(count / 2);
	}

	bool rgc_replace_edge(long id, char const *result_name,
						char const *recipient_name, int w_1, int w_2,
						char const *donor_name, int v_1, int v_2) {
		auto *recipient_graph = find_graph(id, recipient_name);
		auto *donor_graph = find_graph(id, donor_name);
			
		if (recipient_graph != nullptr && donor_graph != nullptr 
			&& is_name_correct(result_name)) {

			auto *result_graph = find_graph(id, result_name);
			auto *w1_neighbours = find_neighbours(id, recipient_name, w_1);
			auto edge_it = w1_neighbours->find(w_2);
			auto v1_it = donor_graph->find(v_1);
			auto v2_it = donor_graph->find(v_2);

			ssize_t nodes_recipient = 
				static_cast<ssize_t>(recipient_graph->size());
			ssize_t nodes_donor =
				static_cast<ssize_t>(donor_graph->size());

			if (result_graph == nullptr && w1_neighbours != nullptr &&
				edge_it != w1_neighbours->end() && v1_it != donor_graph->end() &&
				v2_it != donor_graph->end() && v_1 != v_2 &&
				nodes_recipient <= INT_MAX - (nodes_donor - 2)) {

				ssize_t nodes_result = nodes_recipient + (nodes_donor - 2);
				ssize_t replaced_edge_level = edge_it->second;
				ssize_t donor_level =
					rgc_graph_level(id, donor_name);
				ssize_t recipient_without_replaced_edge_level =
					rgc_graph_level_without_edge(id, recipient_name, w_1, w_2);
				ssize_t result_level = recipient_without_replaced_edge_level;
				ssize_t new_edge_level = 0;

				if (rgc_count_edges(id, donor_name) > 0) {
					// Warunek na poziom grafu wynikowego.
					if (max(donor_level, replaced_edge_level) < LEVEL_MAX - 1) {
						new_edge_level = max(donor_level, replaced_edge_level) + 1;
						result_level = max(recipient_without_replaced_edge_level,
										   new_edge_level);
					}
					else {
						return false;
					}
				}

				// tutaj zaczynamy tworzenie grafu wynikowego.
				return true;
			}
		}

		return false;
	}	// Oliwier

}  // namespace cxx
