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

#include "emu/framebuffer.hpp"
#include <cstring>

framebuffer::framebuffer() : _width{0}, _height{0} {
}

framebuffer::framebuffer(std::size_t width, std::size_t height)
    : _width{width}, _height{height}, _pixels(_width * _height, 0) {
}

void framebuffer::clear() {
  std::memset(_pixels.data(), 0, _pixels.size());
}

bool framebuffer::toggle(int x, int y) {
  wrap(x, y);
  return _pixels[y * _width + x] ^= 255;
}

bool framebuffer::is_on(int x, int y) const {
  wrap(x, y);
  return _pixels[y * _width + x];
}

inline void framebuffer::wrap(int& x, int& y) const {
  x %= _width;
  y %= _height;
}