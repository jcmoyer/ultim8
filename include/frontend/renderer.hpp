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

#ifndef FRONTEND_RENDERER_HPP
#define FRONTEND_RENDERER_HPP

#include <GL/gl3w.h>
#include "frontend/color.hpp"

class chip8vm;

class renderer {
public:
  renderer();
  ~renderer();

  void render(const chip8vm* chip8);

  void set_background_color(const color4f& c) { background = c; }
  void set_foreground_color(const color4f& c) { foreground = c; }

private:
  GLuint vertex_buffer = 0;
  GLuint vertex_shader = 0;
  GLuint fragment_shader = 0;
  GLuint shader_prog = 0;
  GLuint emu_texture = 0;
  GLuint vertex_array = 0;
  GLint background_color = 0;
  GLint foreground_color = 0;

  struct output_dimensions {
    int width, height;
  };

  output_dimensions dims;
  color4f background;
  color4f foreground;
};

#endif