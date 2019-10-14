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

#ifndef EMU_INPUT_HPP
#define EMU_INPUT_HPP

#include <cassert>
#include <array>

enum chip8_key {
  HEXKEY_0,
  HEXKEY_1,
  HEXKEY_2,
  HEXKEY_3,
  HEXKEY_4,
  HEXKEY_5,
  HEXKEY_6,
  HEXKEY_7,
  HEXKEY_8,
  HEXKEY_9,
  HEXKEY_A,
  HEXKEY_B,
  HEXKEY_C,
  HEXKEY_D,
  HEXKEY_E,
  HEXKEY_F,
  HEXKEY_COUNT
};

inline constexpr bool is_valid_key(chip8_key k) {
  return k >= 0 && k < HEXKEY_COUNT;
}

struct input_state {
  static const chip8_key LAST_KEY_NONE = (chip8_key)-1;

  std::array<bool, HEXKEY_COUNT> keys{0};
  chip8_key last_key = LAST_KEY_NONE;

  void set_key_state(chip8_key k, bool state) {
    assert(is_valid_key(k));
    keys[k] = state;
    if (state) {
      last_key = k;
    }
  }

  bool is_pressed(chip8_key k) const {
    assert(is_valid_key(k));
    return keys[k];
  }

  void clear() {
    clear_last_key();
    keys.fill(false);
  }

  void clear_last_key() { last_key = LAST_KEY_NONE; }
  bool has_last_key() const { return last_key != LAST_KEY_NONE; }
};

#endif
