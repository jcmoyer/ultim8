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

#ifndef FRONTEND_COLOR_HPP
#define FRONTEND_COLOR_HPP

#include <cstdint>

struct color4i {
  uint8_t r, g, b, a;

  color4i() : r{0}, g{0}, b{0}, a{255} {}
  color4i(uint8_t r_, uint8_t g_, uint8_t b_) : r{r_}, g{g_}, b{b_}, a{255} {}
  color4i(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r{r_}, g{g_}, b{b_}, a{a_} {}
};

struct color4f {
  float r, g, b, a;

  color4f() : r{0}, g{0}, b{0}, a{1} {}

  color4f(float r_, float g_, float b_, float a_) : r{r_}, g{g_}, b{b_}, a{a_} {}

  color4f(const color4i& src) {
    r = src.r / 255.f;
    g = src.g / 255.f;
    b = src.b / 255.f;
    a = src.a / 255.f;
  }
};

#endif