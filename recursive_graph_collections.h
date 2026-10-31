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

// These functions create, delete, and query collections.
long rgc_new_collection(void);
void rgc_delete_collection(long id);
ssize_t rgc_size(long id);

// These functions create and delete graphs within a collection.
bool rgc_new_graph(long id, char const *name, int n);
void rgc_delete_graph(long id, char const *name);

// These functions add and remove vertices and edges.
int rgc_graph_new_node(long id, char const *name);
bool rgc_graph_delete_node(long id, char const *name, int x);
bool rgc_graph_add_edge(long id, char const *name, int x, int y);
bool rgc_graph_delete_edge(long id, char const *name, int x, int y);

// These functions iterate over graphs, vertices, and neighbours in ascending
// order. Returned names remain valid until their graph or collection is
// deleted.
char const *rgc_first_graph(long id);
char const *rgc_next_graph(long id, char const *name);
int rgc_first_node(long id, char const *name);
int rgc_next_node(long id, char const *name, int x);
int rgc_first_edge(long id, char const *name, int x);
int rgc_next_edge(long id, char const *name, int x, int y);

// This function creates a graph by replacing a recipient edge with the donor
// graph.
bool rgc_replace_edge(long id, char const *result_name,
					  char const *recipient_name, int w_1, int w_2,
					  char const *donor_name, int v_1, int v_2);

// These functions query levels and count vertices and edges.
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
