// Copyright 2019 J.C. Moyer
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COMMON_INTEGRAL_CAST_HPP
#define COMMON_INTEGRAL_CAST_HPP

#include <limits>
#include <stdexcept>
#include <type_traits>

struct bad_integral_cast : std::runtime_error {
  using std::runtime_error::runtime_error;
};

template <typename To, typename From>
inline constexpr To integral_cast(From x) {
  static_assert(std::is_integral_v<From> && std::is_integral_v<To>,
    "integral_cast only supports integral types");

  using larger_type = std::conditional_t<(sizeof(From) > sizeof(To)), From, To>;
  using unsigned_type = std::make_unsigned_t<larger_type>;
  using signed_type = std::make_signed_t<larger_type>;

  if constexpr (std::numeric_limits<From>::is_signed == std::numeric_limits<To>::is_signed) {
    if (static_cast<larger_type>(x) > static_cast<larger_type>(std::numeric_limits<To>::max())) {
      throw bad_integral_cast("source value out of range for destination type");
    }

    if (static_cast<larger_type>(x) < static_cast<larger_type>(std::numeric_limits<To>::lowest())) {
      throw bad_integral_cast("source value out of range for destination type");
    }
  } else if constexpr (std::numeric_limits<From>::is_signed && !std::numeric_limits<To>::is_signed) {
    if (x < 0) {
      throw bad_integral_cast("source value is negative but destination type is unsigned");
    }

    if (static_cast<unsigned_type>(x) > static_cast<unsigned_type>(std::numeric_limits<To>::max())) {
      throw bad_integral_cast("source value out of range for destination type");
    }
  } else if constexpr (!std::numeric_limits<From>::is_signed && std::numeric_limits<To>::is_signed) {
    if (static_cast<signed_type>(x) < 0) {
      throw bad_integral_cast("overflow");
    }

    if (static_cast<signed_type>(x) > static_cast<signed_type>(std::numeric_limits<To>::max())) {
      throw bad_integral_cast("source value out of range for destination type");
    }
  }

  return static_cast<To>(x);
}

#endif
