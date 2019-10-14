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

#include "frontend/config.hpp"
#include "frontend/color_toml.hpp"
#include "frontend/keymap.hpp"
#include "common/integral_cast.hpp"
#include <toml.hpp>

void load_input_config(const toml::value& n, input_config& input) {
  // TODO: clean this mess up, probably want another X macro
  // it is unlikely 0..f as keynames will stick since we need to
  // allow re/unbinding emulator controls (debug step, reload, etc.)
  input.kmap[to_sdl_key(n.at("0").as_string())] = HEXKEY_0;
  input.kmap[to_sdl_key(n.at("1").as_string())] = HEXKEY_1;
  input.kmap[to_sdl_key(n.at("2").as_string())] = HEXKEY_2;
  input.kmap[to_sdl_key(n.at("3").as_string())] = HEXKEY_3;
  input.kmap[to_sdl_key(n.at("4").as_string())] = HEXKEY_4;
  input.kmap[to_sdl_key(n.at("5").as_string())] = HEXKEY_5;
  input.kmap[to_sdl_key(n.at("6").as_string())] = HEXKEY_6;
  input.kmap[to_sdl_key(n.at("7").as_string())] = HEXKEY_7;
  input.kmap[to_sdl_key(n.at("8").as_string())] = HEXKEY_8;
  input.kmap[to_sdl_key(n.at("9").as_string())] = HEXKEY_9;
  input.kmap[to_sdl_key(n.at("a").as_string())] = HEXKEY_A;
  input.kmap[to_sdl_key(n.at("b").as_string())] = HEXKEY_B;
  input.kmap[to_sdl_key(n.at("c").as_string())] = HEXKEY_C;
  input.kmap[to_sdl_key(n.at("d").as_string())] = HEXKEY_D;
  input.kmap[to_sdl_key(n.at("e").as_string())] = HEXKEY_E;
  input.kmap[to_sdl_key(n.at("f").as_string())] = HEXKEY_F;

  input.reload = to_sdl_key(n.at("reload").as_string());
  input.toggle_debugger = to_sdl_key(n.at("toggle_debugger").as_string());
  input.increase_cycles = to_sdl_key(n.at("increase_cycles").as_string());
  input.decrease_cycles = to_sdl_key(n.at("decrease_cycles").as_string());
}

void load_audio_config(const toml::value& n, audio_config& audio) {
  audio.frequency = integral_cast<int>(n.at("frequency").as_integer());
  audio.samples = integral_cast<int>(n.at("samples").as_integer());
}

void load_display_config(const toml::value& n, display_config& display) {
  display.render_background = toml::get<color4i>(n.at("background"));
  display.render_foreground = toml::get<color4i>(n.at("foreground"));
}

void load_debug_config(const toml::value& n, debug_config& debug) {
  debug.visible = n.at("visible").as_boolean();
}

application_config load_config(const std::string& path) {
  application_config cfg;

  toml::value data;
  try {
    data = toml::parse(path);
  } catch (const toml::syntax_error& e) {
    throw config_error("syntax error in config file", e.what());
  }
  
  try {
    load_input_config(data.at("input"), cfg.input);
    load_audio_config(data.at("audio"), cfg.audio);
    load_display_config(data.at("display"), cfg.display);
    load_debug_config(data.at("debug"), cfg.debug);
  } catch (const toml::type_error& e) {
    throw config_error("config setting has an incorrect type", e.what());
  } catch (const bad_integral_cast& e) {
    throw config_error("config setting is out of range", e.what());
  }
  
  return cfg;
}