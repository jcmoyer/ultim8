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

  #define KEY_FOR(name) to_sdl_key(static_cast<std::string_view>(n.at(name).as_string()))

  input.kmap[KEY_FOR("0")] = HEXKEY_0;
  input.kmap[KEY_FOR("1")] = HEXKEY_1;
  input.kmap[KEY_FOR("2")] = HEXKEY_2;
  input.kmap[KEY_FOR("3")] = HEXKEY_3;
  input.kmap[KEY_FOR("4")] = HEXKEY_4;
  input.kmap[KEY_FOR("5")] = HEXKEY_5;
  input.kmap[KEY_FOR("6")] = HEXKEY_6;
  input.kmap[KEY_FOR("7")] = HEXKEY_7;
  input.kmap[KEY_FOR("8")] = HEXKEY_8;
  input.kmap[KEY_FOR("9")] = HEXKEY_9;
  input.kmap[KEY_FOR("a")] = HEXKEY_A;
  input.kmap[KEY_FOR("b")] = HEXKEY_B;
  input.kmap[KEY_FOR("c")] = HEXKEY_C;
  input.kmap[KEY_FOR("d")] = HEXKEY_D;
  input.kmap[KEY_FOR("e")] = HEXKEY_E;
  input.kmap[KEY_FOR("f")] = HEXKEY_F;

  input.reload = KEY_FOR("reload");
  input.toggle_debugger = KEY_FOR("toggle_debugger");
  input.increase_cycles = KEY_FOR("increase_cycles");
  input.decrease_cycles = KEY_FOR("decrease_cycles");
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