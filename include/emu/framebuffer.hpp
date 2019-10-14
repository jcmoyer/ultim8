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

#ifndef CHIP8_FRAMEBUFFER_HPP
#define CHIP8_FRAMEBUFFER_HPP

#include <vector>
#include <cstdint>

class framebuffer {
public:
  framebuffer();
  framebuffer(std::size_t width, std::size_t height);

  void clear();
  bool toggle(int x, int y);
  bool is_on(int x, int y) const;

  std::size_t width() const { return _width; }
  std::size_t height() const { return _height; }
  const uint8_t* data() const { return _pixels.data(); }

private:
  void wrap(int& x, int& y) const;

  std::size_t _width;
  std::size_t _height;
  std::vector<uint8_t> _pixels;
};

#endif