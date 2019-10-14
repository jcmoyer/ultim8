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

#ifndef FRONTEND_CONFIG_HPP
#define FRONTEND_CONFIG_HPP

#include <stdexcept>
#include <map>
#include <SDL_keycode.h>
#include "emu/input.hpp"
#include "frontend/color.hpp"
#include "keymap.hpp"

class config_error : public std::runtime_error {
public:
  config_error(const char* msg, std::string context_)
      : std::runtime_error(msg), context{std::move(context_)} {}
  std::string context;
};

struct audio_config {
  int frequency = 48000;
  int samples = 512;
};

struct input_config {
  keymap kmap;

  SDL_Keycode reload;
  SDL_Keycode toggle_debugger;
  SDL_Keycode increase_cycles;
  SDL_Keycode decrease_cycles;
};

struct display_config {
  color4i render_foreground{0xac, 0xd5, 0xff};
  color4i render_background{0x11, 0x31, 0x52};
};

struct debug_config {
  bool visible = false;
};

struct application_config {
  audio_config audio;
  input_config input;
  display_config display;
  debug_config debug;
};

application_config load_config(const std::string& path);

#endif