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

#ifndef FRONTEND_KEYMAP_HPP
#define FRONTEND_KEYMAP_HPP

#include <map>
#include <stdexcept>
#include <string>
#include <SDL_keycode.h>
#include "emu/input.hpp"

struct bad_key_name : std::runtime_error {
  bad_key_name(const char* what, std::string keyname_)
      : std::runtime_error(what), keyname{keyname_} {}
  std::string keyname;
};

using keymap = std::map<SDL_Keycode, chip8_key>;

SDL_Keycode to_sdl_key(std::string_view keyname);

#endif