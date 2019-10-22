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

#ifndef FRONTEND_COLOR_TOML_HPP
#define FRONTEND_COLOR_TOML_HPP

#include <toml.hpp>
#include "frontend/color.hpp"
#include "common/integral_cast.hpp"

namespace toml {
template <>
struct from<color4i> {
  static color4i from_toml(const toml::value& v) {
    color4i c;
    c.r = integral_cast<uint8_t>(v.at(0).as_integer());
    c.g = integral_cast<uint8_t>(v.at(1).as_integer());
    c.b = integral_cast<uint8_t>(v.at(2).as_integer());
    if (v.as_array().size() == 4) {
      c.a = integral_cast<uint8_t>(v.at(3).as_integer());
    } else {
      c.a = 255;
    }
    return c;
  }
};
} // namespace toml

#endif