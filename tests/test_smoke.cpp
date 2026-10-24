#include <catch2/catch_test_macros.hpp>

static_assert(__cplusplus >= 202002L, "pastar requires C++20");

TEST_CASE("Test pipeline works", "[smoke]") { REQUIRE(1 + 1 == 2); }
