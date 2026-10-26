#include <pastar/graph.hpp>

#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace pastar;

namespace {
using Adj = std::vector<std::pair<NodeId, Weight>>;

// Out-edges of u as (to, w) pairs, so Catch2 can print them on failure.
Adj adj(const Graph &g, NodeId u) {
  Adj out;
  for (const Edge &e : g.neighbors(u)) {
    out.emplace_back(e.to, e.w);
  }
  return out;
}
} // namespace

TEST_CASE("Hand-made 5-node graph from unordered input", "[graph]") {
  //  0 -> 1 (1), 0 -> 2 (4), 1 -> 2 (2), 2 -> 3 (1), 3 -> 0 (7); node 4 isolated
  const std::vector<EdgeInput> in = {
      {2, 3, 1.0f}, {0, 1, 1.0f}, {3, 0, 7.0f}, {1, 2, 2.0f}, {0, 2, 4.0f},
  };
  const Graph g = build_graph(5, in);

  REQUIRE(g.num_nodes() == 5);
  REQUIRE(g.num_edges() == 5);
  REQUIRE(check_invariants(g));

  CHECK(adj(g, 0) == Adj{{1, 1.0f}, {2, 4.0f}});
  CHECK(adj(g, 1) == Adj{{2, 2.0f}});
  CHECK(adj(g, 2) == Adj{{3, 1.0f}});
  CHECK(adj(g, 3) == Adj{{0, 7.0f}});
  CHECK(adj(g, 4).empty());

  const std::vector<EdgeId> expected_offsets = {0, 2, 3, 4, 5, 5};
  CHECK(std::vector<EdgeId>(g.offsets().begin(), g.offsets().end()) == expected_offsets);
}

TEST_CASE("Out-edges keep their input order (stable build)", "[graph]") {
  const std::vector<EdgeInput> in = {
      {0, 3, 1.0f},
      {1, 0, 1.0f},
      {0, 1, 2.0f},
      {0, 2, 3.0f},
  };
  const Graph g = build_graph(4, in);
  CHECK(adj(g, 0) == Adj{{3, 1.0f}, {1, 2.0f}, {2, 3.0f}});
}

TEST_CASE("Nodes without out-edges, including the first and the last", "[graph]") {
  const std::vector<EdgeInput> in = {{1, 2, 1.0f}, {2, 1, 1.0f}};
  const Graph g = build_graph(4, in);

  REQUIRE(check_invariants(g));
  CHECK(adj(g, 0).empty());
  CHECK(adj(g, 1) == Adj{{2, 1.0f}});
  CHECK(adj(g, 2) == Adj{{1, 1.0f}});
  CHECK(adj(g, 3).empty()); // last node: classic off-by-one in the prefix sum
  CHECK(g.offsets().back() == 2);
}

TEST_CASE("Empty graphs", "[graph]") {
  SECTION("default-constructed") {
    const Graph g;
    CHECK(g.num_nodes() == 0);
    CHECK(g.num_edges() == 0);
    CHECK(g.offsets().size() == 1);
    CHECK(check_invariants(g));
  }
  SECTION("zero nodes") {
    const Graph g = build_graph(0, {});
    CHECK(g.num_nodes() == 0);
    CHECK(g.offsets().size() == 1);
    CHECK(check_invariants(g));
  }
  SECTION("nodes but no edges") {
    const Graph g = build_graph(3, {});
    CHECK(g.num_nodes() == 3);
    CHECK(g.num_edges() == 0);
    CHECK(g.offsets().size() == 4);
    CHECK(check_invariants(g));
    for (NodeId u = 0; u < 3; ++u) {
      CHECK(adj(g, u).empty());
    }
  }
}

TEST_CASE("Self-loops and parallel edges are kept", "[graph]") {
  const std::vector<EdgeInput> in = {{0, 0, 1.0f}, {0, 1, 5.0f}, {0, 1, 3.0f}};
  const Graph g = build_graph(2, in);
  CHECK(adj(g, 0) == Adj{{0, 1.0f}, {1, 5.0f}, {1, 3.0f}});
  CHECK(check_invariants(g));
}

TEST_CASE("Zero-weight edges are accepted", "[graph]") {
  const std::vector<EdgeInput> in = {{0, 1, 0.0f}};
  CHECK_NOTHROW(build_graph(2, in));
}

TEST_CASE("Invalid input is rejected", "[graph]") {
  const float nan = std::numeric_limits<float>::quiet_NaN();
  using In = std::vector<EdgeInput>;

  CHECK_THROWS_AS(build_graph(2, In{{0, 2, 1.0f}}), std::invalid_argument); // to >= n
  CHECK_THROWS_AS(build_graph(2, In{{2, 0, 1.0f}}), std::invalid_argument); // from >= n
  CHECK_THROWS_AS(build_graph(0, In{{0, 0, 1.0f}}), std::invalid_argument); // any edge, n = 0
  CHECK_THROWS_AS(build_graph(2, In{{0, 1, -1.0f}}), std::invalid_argument);
  CHECK_THROWS_AS(build_graph(2, In{{0, 1, nan}}), std::invalid_argument);
  CHECK_THROWS_AS(build_graph(2, In{{0, 1, kInfinity}}), std::invalid_argument);
}
