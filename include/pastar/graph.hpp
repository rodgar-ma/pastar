#pragma once

#include <pastar/types.hpp>

#include <span>
#include <utility>
#include <vector>

namespace pastar {

struct EdgeInput {
  NodeId from;
  NodeId to;
  Weight w;
};

/// Immutable directed graph in CSR (compressed sparse row) form.
///
/// The out-edges of node u are edges()[offsets()[u] .. offsets()[u + 1]).
/// Only topology and weights: coordinates for heuristics live elsewhere.
/// Immutable after construction, so it can be shared between threads
/// without locks and copied to the GPU as two flat arrays.
class Graph {
public:
  Graph() = default;

  [[nodiscard]] NodeId num_nodes() const noexcept {
    return static_cast<NodeId>(offsets_.size() - 1);
  }

  [[nodiscard]] EdgeId num_edges() const noexcept { return static_cast<EdgeId>(edges_.size()); }

  [[nodiscard]] std::span<const Edge> neighbors(NodeId u) const noexcept {
    const Edge *base = edges_.data();
    return {base + offsets_[u], base + offsets_[u + 1]};
  }

  [[nodiscard]] std::span<const EdgeId> offsets() const noexcept { return offsets_; }
  [[nodiscard]] std::span<const Edge> edges() const noexcept { return edges_; }

private:
  friend Graph build_graph(NodeId num_nodes, std::span<const EdgeInput> input);

  Graph(std::vector<EdgeId> offsets, std::vector<Edge> edges)
      : offsets_(std::move(offsets)), edges_(std::move(edges)) {}

  std::vector<EdgeId> offsets_{0};
  std::vector<Edge> edges_;
};

/// Builds a CSR graph from an edge list with a stable counting sort, O(n + m).
///
/// - Stable: the out-edges of each node keep their input order. Neighbour
///   order decides heap insertion order and therefore tie-breaking, so this
///   keeps expanded-node counts reproducible.
/// - Directed: an undirected edge must be given as two edges (readers do it).
/// - Self-loops and parallel edges are kept.
///
/// Throws std::invalid_argument if an endpoint is >= num_nodes, a weight is
/// negative, NaN or infinite, or the edge count does not fit in EdgeId.
/// Node ids are 0 .. num_nodes - 1, so kInvalidNode is never a valid id.
[[nodiscard]] Graph build_graph(NodeId num_nodes, std::span<const EdgeInput> input);

/// True if g satisfies the CSR invariants (used by tests and asserts).
[[nodiscard]] bool check_invariants(const Graph &g) noexcept;

} // namespace pastar
