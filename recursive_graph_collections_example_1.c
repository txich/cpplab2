#include "recursive_graph_collections.h"

#ifdef NDEBUG
  #undef NDEBUG
#endif

#include <assert.h>
#include <stdio.h>

int main() {
  long id = rgc_new_collection();
  assert(id == 0);

  // Graph G_0 is a path with vertices 0 – 1 – 2.
  assert(rgc_new_graph(id, "G_0", 3));
  assert(rgc_graph_add_edge(id, "G_0", 0, 1));
  assert(rgc_graph_add_edge(id, "G_0", 1, 2));

  // Graph G_{k+1} is formed from G_k by replacing the edge incident to 0
  // with G_k itself, using 0 and 2 as the selected endpoints. The result is
  // still a path, and each replacement increases the graph level by 1.
  char name[16], next[16];
  int k = 0, n = 3;
  for (;; k++, n = 2 * n - 2) {
    snprintf(name, sizeof name, "G_%d", k);
    snprintf(next, sizeof next, "G_%d", k + 1);
    int x = rgc_first_edge(id, name, 0);
    if (!rgc_replace_edge(id, next, name, 0, x, name, 0, 2))
      break;
    assert(rgc_count_nodes(id, next) == 2 * n - 2);
    assert(rgc_count_edges(id, next) == 2 * n - 3);
    assert(rgc_graph_level(id, next) == k + 1);
    assert(rgc_edge_level(id, next, 1, 2) == 0);
  }

  // No further replacement is possible, as the level would exceed the maximum.
  assert(k == rgc_max_level());
  assert(rgc_graph_level(id, name) == rgc_max_level());
  assert(rgc_count_nodes(id, next) == -1);
  assert(rgc_size(id) == k + 1);

  // Edge {1, 2} still has level 0, so it can be replaced with a donor whose level
  // is at most rgc_max_level() - 1, such as graph G_0, but not graph G_k.
  assert(!rgc_replace_edge(id, "Q", name, 1, 2, name, 0, 2));
  assert(rgc_replace_edge(id, "Q", name, 1, 2, "G_0", 0, 2));
  assert(rgc_edge_level(id, "Q", 1, 2) == -1);
  assert(rgc_graph_level(id, "Q") == rgc_max_level());

  rgc_delete_collection(id);
  assert(rgc_size(id) == -1);
}
