#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

// The code relies on IEEE-754 infinities and NaNs (kInfinity, weights_equal).
// -ffast-math / -ffinite-math-only let the compiler assume they never occur.
#if defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__
#error "pastar must not be compiled with -ffast-math or -ffinite-math-only"
#endif

namespace pastar {

using NodeId = std::uint32_t;
inline constexpr NodeId kInvalidNode = std::numeric_limits<NodeId>::max();

// Index into the CSR edge array. 32 bits cover DIMACS USA (~58 M edges);
// the graph builder must reject graphs with more edges than this type holds.
using EdgeId = std::uint32_t;

using Weight = float;
inline constexpr Weight kInfinity = std::numeric_limits<Weight>::infinity();

static_assert(std::numeric_limits<Weight>::is_iec559, "Weights type must be IEC559 compliant.");
static_assert(std::numeric_limits<Weight>::has_infinity, "Weights type must have an infinity.");

// Default relative tolerance for comparing path costs.
// Provisional: to be fixed from measured float error in milestone M5 (decision A7).
inline constexpr double kDefaultTol = 1e-5;

struct Edge {
  NodeId to;
  Weight w;
};

static_assert(sizeof(Edge) == 8, "Edge must be 8 bytes.");
static_assert(std::is_trivially_copyable_v<Edge>, "Edge must be trivially copyable (GPU copy).");

/// Relative comparison of path costs.
///
/// Semantics (each case is deliberate):
///   - equal values (including inf == inf, 0 == 0) -> true
///   - any NaN                                      -> false
///   - inf vs finite                                -> false
///   - otherwise |a - b| <= rel_tol * max(|a|, |b|)
/// Purely relative: 0 vs 1e-30 is false (cost 0 only for source == target).
template <std::floating_point T>
[[nodiscard]] bool weights_equal(T a, T b, T rel_tol = static_cast<T>(kDefaultTol)) {
  if (a == b) {
    return true;
  }
  if (std::isnan(a) || std::isnan(b)) {
    return false;
  }
  if (std::isinf(a) || std::isinf(b)) {
    return false;
  }
  return std::fabs(a - b) <= rel_tol * std::max(std::fabs(a), std::fabs(b));
}

} // namespace pastar
