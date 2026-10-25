#ifndef RECURSIVE_GRAPH_COLLECTIONS_H
#define RECURSIVE_GRAPH_COLLECTIONS_H

#include <sys/types.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
namespace cxx {
extern "C" {
#endif

// Zarządzanie kolekcjami.
long rgc_new_collection(void);
void rgc_delete_collection(long id);
ssize_t rgc_size(long id);

// Zarządzanie grafami.
bool rgc_new_graph(long id, char const *name, int n);
void rgc_delete_graph(long id, char const *name);

// Modyfikowanie wierzchołków i krawędzi.
int rgc_graph_new_node(long id, char const *name);
bool rgc_graph_delete_node(long id, char const *name, int x);
bool rgc_graph_add_edge(long id, char const *name, int x, int y);
bool rgc_graph_delete_edge(long id, char const *name, int x, int y);

// Przeglądanie grafów, wierzchołków i sąsiadów w porządku rosnącym.
char const *rgc_first_graph(long id);
char const *rgc_next_graph(long id, char const *name);
int rgc_first_node(long id, char const *name);
int rgc_next_node(long id, char const *name, int x);
int rgc_first_edge(long id, char const *name, int x);
int rgc_next_edge(long id, char const *name, int x, int y);

// Tworzenie grafu przez zastąpienie krawędzi biorcy grafem dawcy.
bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2);

// Poziomy oraz liczby wierzchołków i krawędzi.
ssize_t rgc_edge_level(long id, char const *name, int x, int y);
ssize_t rgc_graph_level(long id, char const *name);
ssize_t rgc_max_level(void);
ssize_t rgc_count_nodes(long id, char const *name);
ssize_t rgc_count_edges(long id, char const *name);

#ifdef __cplusplus
}  // extern "C"
}  // namespace cxx
#endif

#endif	// RECURSIVE_GRAPH_COLLECTIONS_H
