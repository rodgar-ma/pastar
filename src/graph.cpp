#include <pastar/graph.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pastar {

Graph build_graph(NodeId num_nodes, std::span<const EdgeInput> input) {
  if (input.size() > std::numeric_limits<EdgeId>::max()) {
    throw std::invalid_argument("build_graph: " + std::to_string(input.size()) +
                                " edges do not fir in EdgeId");
  }

  const std::size_t n = num_nodes;

  std::vector<EdgeId> offsets(n + 1, 0);
  for (std::size_t i = 0; i < input.size(); i++) {
    const EdgeInput &e = input[i];
    if (e.from >= num_nodes || e.to >= num_nodes) {
      throw std::invalid_argument("build_graph: edge " + std::to_string(i) +
                                  " has an endpoint >= num_nodes");
    }
    if (!(e.w >= 0.0f) || std::isinf(e.w)) {
      throw std::invalid_argument("build_graph: edge " + std::to_string(i) +
                                  " has a negative, NaN or infinite weight");
    }
    offsets[std::size_t{e.from} + 1]++;
  }

  for (std::size_t u = 0; u < n; u++) {
    offsets[u + 1] += offsets[u];
  }

  std::vector<EdgeId> cursor(offsets.begin(), offsets.end() - 1);
  std::vector<Edge> edges(input.size());
  for (const EdgeInput &e : input) {
    edges[cursor[e.from]] = Edge{e.to, e.w};
    cursor[e.from]++;
  }

  Graph g(std::move(offsets), std::move(edges));
  assert(check_invariants(g));
  return g;
}

bool check_invariants(const Graph &g) noexcept {
  const auto offsets = g.offsets();
  const auto edges = g.edges();
  const std::size_t n = g.num_nodes();

  if (offsets.size() != n + 1 || offsets[0] != 0 || offsets[n] != edges.size()) {
    return false;
  }

  for (std::size_t u = 0; u < n; u++) {
    if (offsets[u] > offsets[u + 1]) {
      return false;
    }
  }

  for (const Edge &e : edges) {
    if (e.to >= g.num_nodes()) {
      return false;
    }
  }

  return true;
}

} // namespace pastar
