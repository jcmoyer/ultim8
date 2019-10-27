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

#include "frontend/keymap.hpp"
#include <unordered_map>

SDL_Keycode to_sdl_key(std::string_view s) {
  // yes, macros - very spooky
  const static std::unordered_map<std::string_view, SDL_Keycode> sdl_key_map{
#define KEY(name) {"" #name, name},
#include "frontend/keys.def"
  };

  if (auto it = sdl_key_map.find(s); it != sdl_key_map.end()) {
    return it->second;
  } else {
    throw bad_key_name("invalid key", std::string(s));
  }
}
