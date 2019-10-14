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

#include "frontend/renderer.hpp"
#include "emu/vm.hpp"

const char* vertex_shader_src = R"(
#version 330 core
layout(location = 0) in vec4 aPos;
out vec2 TexCoord;

void main() {
  gl_Position = aPos;
  TexCoord.x = (1 + aPos.x) / 2.0f;
  TexCoord.y = 1 - ((1 + aPos.y) / 2.0f);
}
)";

const char* fragment_shader_src = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

uniform vec4 foreground_color;
uniform vec4 background_color;

void main() {
  FragColor = mix(
    background_color,
    foreground_color,
    texture(ourTexture, TexCoord).r
  );
} 
)";

renderer::renderer() {
  background = {0x0f / 255.f, 0x38 / 255.f, 0x0f / 255.f, 1};
  foreground = {0x8b / 255.f, 0xac / 255.f, 0x0f / 255.f, 1};

  float screen_quad[] = {-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1};
  
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(screen_quad), screen_quad, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_src, 0);
  glCompileShader(vertex_shader);

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src, 0);
  glCompileShader(fragment_shader);

  shader_prog = glCreateProgram();
  glAttachShader(shader_prog, vertex_shader);
  glAttachShader(shader_prog, fragment_shader);
  glLinkProgram(shader_prog);

  foreground_color = glGetUniformLocation(shader_prog, "foreground_color");
  background_color = glGetUniformLocation(shader_prog, "background_color");

  glGenTextures(1, &emu_texture);
  glBindTexture(GL_TEXTURE_2D, emu_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  dims.width = 0;
  dims.height = 0;
}

renderer::~renderer() {
  glDeleteTextures(1, &emu_texture);
  glDeleteProgram(shader_prog);
  glDeleteShader(fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteVertexArrays(1, &vertex_array);
}

void renderer::render(const chip8vm* chip8) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, emu_texture);

  // if output dimensions changed, we have to reallocate texture storage using new dimensions
  int fbwidth = static_cast<int>(chip8->framebuf.width());
  int fbheight = static_cast<int>(chip8->framebuf.height());
  if (fbwidth != dims.width || fbheight != dims.height) {
    dims.width = fbwidth;
    dims.height = fbheight;
    glTexImage2D(GL_TEXTURE_2D,
      0,
      GL_RGB,
      dims.width,
      dims.height,
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      chip8->framebuf.data());
  } else {
    glTexSubImage2D(GL_TEXTURE_2D,
      0,
      0,
      0,
      dims.width,
      dims.height,
      GL_RED,
      GL_UNSIGNED_BYTE,
      chip8->framebuf.data());
  }

  glUseProgram(shader_prog);
  glUniform4fv(background_color, 1, (GLfloat*)&background);
  glUniform4fv(foreground_color, 1, (GLfloat*)&foreground);

  glBindVertexArray(vertex_array);
  
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
