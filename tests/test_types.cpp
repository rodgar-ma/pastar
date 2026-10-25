#include <pastar/types.hpp>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <type_traits>

using namespace pastar;

namespace {
// Shorthand so the test values read the same for float and double.
template <typename T> constexpr T v(double x) { return static_cast<T>(x); }
template <typename T> constexpr T inf() { return std::numeric_limits<T>::infinity(); }
template <typename T> constexpr T nan() { return std::numeric_limits<T>::quiet_NaN(); }
} // namespace

// ---------------------------------------------------------------------------
// Basic types
// ---------------------------------------------------------------------------
TEST_CASE("Basic types have the expected layout", "[types]") {
  REQUIRE(sizeof(NodeId) == 4);
  REQUIRE(sizeof(EdgeId) == 4);
  REQUIRE(sizeof(Edge) == 8);
  REQUIRE(std::is_trivially_copyable_v<Edge>);
  REQUIRE(kInvalidNode == std::numeric_limits<NodeId>::max());
}

TEST_CASE("kInfinity absorbs additions and compares above any finite weight", "[types]") {
  REQUIRE(kInfinity + 1.0f == kInfinity);
  REQUIRE(std::numeric_limits<Weight>::max() < kInfinity);
}

// ---------------------------------------------------------------------------
// weights_equal (each case instantiated for float and double)
// ---------------------------------------------------------------------------
TEMPLATE_TEST_CASE("weights_equal: exact and infinite values", "[types]", float, double) {
  using T = TestType;
  CHECK(weights_equal(v<T>(1.5), v<T>(1.5)));
  CHECK(weights_equal(v<T>(0.0), v<T>(0.0)));
  CHECK(weights_equal(v<T>(0.0), v<T>(-0.0)));
  CHECK(weights_equal(inf<T>(), inf<T>())); // both "unreachable"
  CHECK_FALSE(weights_equal(inf<T>(), v<T>(5.0)));
  CHECK_FALSE(weights_equal(v<T>(5.0), inf<T>()));
  CHECK_FALSE(weights_equal(-inf<T>(), inf<T>()));
}

TEMPLATE_TEST_CASE("weights_equal: NaN is never equal", "[types]", float, double) {
  using T = TestType;
  CHECK_FALSE(weights_equal(nan<T>(), nan<T>()));
  CHECK_FALSE(weights_equal(nan<T>(), v<T>(1.0)));
  CHECK_FALSE(weights_equal(v<T>(1.0), nan<T>()));
  CHECK_FALSE(weights_equal(nan<T>(), inf<T>()));
}

TEMPLATE_TEST_CASE("weights_equal: tolerance is relative", "[types]", float, double) {
  using T = TestType;
  // Same absolute difference (1.0): equal at 1e7, different at 1.
  CHECK(weights_equal(v<T>(1e7), v<T>(1e7 + 1.0))); // rel. diff 1e-7
  CHECK_FALSE(weights_equal(v<T>(1.0), v<T>(2.0)));
  // Around 1: inside and outside the default tolerance (1e-5).
  CHECK(weights_equal(v<T>(1.0), v<T>(1.000001)));     // rel. diff 1e-6
  CHECK_FALSE(weights_equal(v<T>(1.0), v<T>(1.0001))); // rel. diff 1e-4
  // Purely relative: zero only equals zero.
  CHECK_FALSE(weights_equal(v<T>(0.0), v<T>(1e-30)));
}

TEMPLATE_TEST_CASE("weights_equal: explicit tolerance", "[types]", float, double) {
  using T = TestType;
  CHECK(weights_equal(v<T>(1.5), v<T>(1.5), v<T>(0.0))); // exact values survive tol = 0
  CHECK_FALSE(weights_equal(v<T>(1e7), v<T>(1e7 + 1.0), v<T>(0.0)));
  CHECK(weights_equal(v<T>(1.0), v<T>(1.0001), v<T>(1e-3)));
}

TEMPLATE_TEST_CASE("weights_equal: symmetric", "[types]", float, double) {
  using T = TestType;
  const T values[] = {v<T>(0.0), v<T>(1.0),       v<T>(1.000001), v<T>(1.0001),
                      v<T>(1e7), v<T>(1e7 + 1.0), inf<T>(),       nan<T>()};
  for (T a : values) {
    for (T b : values) {
      CHECK(weights_equal(a, b) == weights_equal(b, a));
    }
  }
}
