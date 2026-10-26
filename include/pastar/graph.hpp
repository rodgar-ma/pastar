#pragma once

#include <pastar/types.hpp>

#include <span>
#include <utility>
#include <vector>

namespace pastar {

typedef struct {
  EdgeId from;
  EdgeId to;
  Weight w;
} EdgeInput;

class Graph {
public:
  Graph() = default;

  [[nodiscard]] NodeId num_nodes() const noexcept {
    return static_cast<NodeId>(offsets_.size() - 1);
  };

  [[nodiscard]] EdgeId num_edges() const noexcept { return static_cast<EdgeId>(edges_.size()); };

  [[nodiscard]] std::span<const Edge> neighbors(NodeId u) const noexcept {
    const Edge *base = edges_.data();
    return {base + offsets_[u], base + offsets_[u + 1]};
  };

  [[nodiscard]] std::span<const EdgeId> offsets() const noexcept { return offsets_; };
  [[nodiscard]] std::span<const Edge> edges() const noexcept { return edges_; };

private:
  friend Graph build_graph(NodeId num_nodes, std::span<const EdgeInput> input);

  Graph(std::vector<EdgeId> offsets, std::vector<Edge> edges)
      : offsets_(std::move(offsets)), edges_(std::move(edges)) {}

  std::vector<EdgeId> offsets_{0};
  std::vector<Edge> edges_;
};

[[nodiscard]] Graph build_graph(NodeId num_nodes, std::span<const EdgeInput> input);

[[nodiscard]] bool check_invariants(const Graph &g) noexcept;

} // namespace pastar
