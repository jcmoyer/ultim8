#include <catch.hpp>
#include <cstdint>
#include "common/integral_cast.hpp"

TEST_CASE("integral_cast") {
  REQUIRE(integral_cast<int8_t, int>(127) == 127);
  REQUIRE_THROWS(integral_cast<int8_t, int>(128));
  REQUIRE(integral_cast<int16_t, int>(128) == 128);
  REQUIRE_THROWS(integral_cast<int16_t, int>(0x8000));
  REQUIRE(integral_cast<int32_t, int32_t>(128) == 128);
  REQUIRE(integral_cast<int32_t, int64_t>(0x7FFFFFFF) == 0x7FFFFFFF);
  REQUIRE_THROWS(integral_cast<int32_t, int64_t>(0x80000000));
  REQUIRE(integral_cast<int32_t, int64_t>(-1) == -1);

  // to bigger type, should always work with same sign
  REQUIRE(integral_cast<int, int8_t>(127) == 127);
  REQUIRE(integral_cast<int, int8_t>(-100) == -100);
  REQUIRE(integral_cast<int, int16_t>(0x7000) == 0x7000);
  REQUIRE(integral_cast<int64_t, int32_t>(0x700000) == 0x700000);

  REQUIRE(integral_cast<uint32_t, uint8_t>(255) == 255);
  REQUIRE(integral_cast<uint32_t, uint8_t>(0) == 0);
  REQUIRE(integral_cast<uint32_t, uint16_t>(0x7000) == 0x7000);
  REQUIRE(integral_cast<uint64_t, uint32_t>(0x700000) == 0x700000);

  // to same type
  REQUIRE(integral_cast<int, int>(127) == 127);
  REQUIRE(integral_cast<int, int>(-100) == -100);
  REQUIRE(integral_cast<uint8_t, uint8_t>(123) == 123);

  // signed/unsigned
  REQUIRE_THROWS(integral_cast<int32_t, uint32_t>(0xFFFFFFFF));
  REQUIRE(integral_cast<int32_t, uint16_t>(0xFFFF) == 0xFFFF);
  REQUIRE(integral_cast<int64_t, uint16_t>(0xFFFF) == 0xFFFF);
  REQUIRE_THROWS(integral_cast<int8_t, uint16_t>(0xFFFF));

  // unsigned/signed
  REQUIRE(integral_cast<uint16_t, int8_t>(127) == 127);
  REQUIRE(integral_cast<uint16_t, int16_t>(0x7FFF) == 0x7FFF);
  REQUIRE_THROWS(integral_cast<uint16_t, int16_t>(-1));
  REQUIRE_THROWS(integral_cast<uint16_t, int32_t>(0x7FFFFFFF));
}