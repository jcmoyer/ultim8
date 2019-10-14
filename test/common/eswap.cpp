#include "common/eswap.hpp"
#include <catch.hpp>
#include <cstdint>

TEST_CASE("eswap") {
  REQUIRE(eswap(0x1234) == 0x3412);
  REQUIRE(eswap(0xF00F) == 0x0FF0);
  REQUIRE(eswap(0x0) == 0x0);
  REQUIRE(eswap(0xFFFF) == 0xFFFF);
  REQUIRE(eswap(0x1111) == 0x1111);
}