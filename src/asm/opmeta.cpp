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

#include "asm/opmeta.hpp"
#include <algorithm>
#include <cassert>

opmetatable::opmetatable() {
  // clang-format off
  _meta = {
    // Core instructions
    {"cls",   operand_type::none, operand_type::none, operand_type::none, 0x00E0, 0, 0, 0},
    {"ret",   operand_type::none, operand_type::none, operand_type::none, 0x00EE, 0, 0, 0},
    {"jmp",   operand_type::addr, operand_type::none, operand_type::none, 0x1000, 0, 0, 0},
    {"call",  operand_type::addr, operand_type::none, operand_type::none, 0x2000, 0, 0, 0},
    {"skeq",  operand_type::v,    operand_type::k,    operand_type::none, 0x3000, 8, 0, 0},
    {"skne",  operand_type::v,    operand_type::k,    operand_type::none, 0x4000, 8, 0, 0},
    {"skeq",  operand_type::v,    operand_type::v,    operand_type::none, 0x5000, 8, 4, 0},
    {"ld",    operand_type::v,    operand_type::k,    operand_type::none, 0x6000, 8, 0, 0},
    {"add",   operand_type::v,    operand_type::k,    operand_type::none, 0x7000, 8, 0, 0},
    {"ld",    operand_type::v,    operand_type::v,    operand_type::none, 0x8000, 8, 4, 0},
    {"or",    operand_type::v,    operand_type::v,    operand_type::none, 0x8001, 8, 4, 0},
    {"and",   operand_type::v,    operand_type::v,    operand_type::none, 0x8002, 8, 4, 0},
    {"xor",   operand_type::v,    operand_type::v,    operand_type::none, 0x8003, 8, 4, 0},
    {"add",   operand_type::v,    operand_type::v,    operand_type::none, 0x8004, 8, 4, 0},
    {"sub",   operand_type::v,    operand_type::v,    operand_type::none, 0x8005, 8, 4, 0},
    {"shr",   operand_type::v,    operand_type::v,    operand_type::none, 0x8006, 8, 4, 0},
    {"subn",  operand_type::v,    operand_type::v,    operand_type::none, 0x8007, 8, 4, 0},
    {"shl",   operand_type::v,    operand_type::v,    operand_type::none, 0x800E, 8, 4, 0},
    {"skne",  operand_type::v,    operand_type::v,    operand_type::none, 0x9000, 8, 4, 0},
    {"ld",    operand_type::i,    operand_type::addr, operand_type::none, 0xA000, 0, 0, 0},
    {"ld",    operand_type::i,    operand_type::k,    operand_type::none, 0xA000, 0, 0, 0},
    {"jmp0",  operand_type::addr, operand_type::none, operand_type::none, 0xB000, 0, 0, 0},
    {"rand",  operand_type::v,    operand_type::k,    operand_type::none, 0xC000, 8, 0, 0},
    {"disp",  operand_type::v,    operand_type::v,    operand_type::k,    0xD000, 8, 4, 0},
    {"skp",   operand_type::v,    operand_type::none, operand_type::none, 0xE09E, 8, 0, 0},
    {"sknp",  operand_type::v,    operand_type::none, operand_type::none, 0xE0A1, 8, 0, 0},
    {"ld",    operand_type::v,    operand_type::dt,   operand_type::none, 0xF007, 8, 0, 0},
    {"input", operand_type::v,    operand_type::none, operand_type::none, 0xF00A, 8, 0, 0},
    {"ld",    operand_type::dt,   operand_type::v,    operand_type::none, 0xF015, 0, 8, 0},
    {"ld",    operand_type::st,   operand_type::v,    operand_type::none, 0xF018, 0, 8, 0},
    {"add",   operand_type::i,    operand_type::v,    operand_type::none, 0xF01E, 0, 8, 0},
    {"glyph", operand_type::v,    operand_type::none, operand_type::none, 0xF029, 8, 0, 0},
    {"bcd",   operand_type::v,    operand_type::none, operand_type::none, 0xF033, 8, 0, 0},
    {"store", operand_type::v,    operand_type::none, operand_type::none, 0xF055, 8, 0, 0},
    {"load",  operand_type::v,    operand_type::none, operand_type::none, 0xF065, 8, 0, 0},

    // superchip instructions
    {"lores", operand_type::none, operand_type::none, operand_type::none, 0x00FE, 0, 0, 0},
    {"hires", operand_type::none, operand_type::none, operand_type::none, 0x00FF, 0, 0, 0},
  };
  // clang-format on

  // needs to be sorted for search algorithms
  std::sort(_meta.begin(), _meta.end());
}

const opmeta* opmetatable::find_signature(const opmeta& sig) const {
  auto lesser = std::lower_bound(std::begin(_meta), std::end(_meta), sig);
  if (lesser != std::end(_meta) && *lesser == sig) {
    return &*lesser;
  } else {
    return nullptr;
  }
}

bool opmetatable::is_mnemonic(std::string_view str) const {
  return std::find_if(std::begin(_meta), std::end(_meta), [=](const auto& m) {
    return m.mnemonic == str;
  }) != std::end(_meta);
}

std::vector<const opmeta*> opmetatable::get_signatures(std::string_view mnemonic) const {
  std::vector<const opmeta*> metas;
  for (const opmeta& m : _meta) {
    if (m.mnemonic == mnemonic)
      metas.push_back(&m);
  }
  return metas;
}

#ifdef _MSC_VER
#include <intrin.h>
#endif

inline uint16_t popcount(uint16_t v) {
#ifdef _MSC_VER
  return __popcnt(v);
#else
  // TODO: Test this on clang and gcc
  return __builtin_popcount(v);
#endif
}

// returns a mask that can be used to remove operands from an opcode
constexpr uint16_t strip_mask(uint16_t opcode) {
  switch ((opcode & 0xF000) >> 12) {
  case 0:
    // NOTE: This is not technically correct...
    // Historically, addresses 0E0 and 0EE point to 1802 subroutines within the original
    // chip-8 interpreter. Emulators are not concerned with emulating 1802, so they treat
    // these opcodes as actual instructions.
    //
    // We don't strip the operand because we handle 00E0 and 00EE as if they were individual
    // instructions.
    return 0xFFFF;
  case 1:
  case 2:
  case 3:
  case 4:
  case 6:
  case 7:
  case 0xA:
  case 0xB:
  case 0xC:
  case 0xD:
    return 0xF000;
  case 5:
  case 8:
  case 9:
    return 0xF00F;
  case 0xE:
  case 0xF:
    return 0xF0FF;
  default:
    // should never hit this, but required to silence warning
    return 0;
  }
}

// returns the number of bits used by operand_type::k for an opcode
size_t freebits(const opmeta& m) {
  uint16_t opcode = strip_mask(m.opcode);
  size_t bits = popcount(~opcode);
  if (m.a == operand_type::v)
    bits -= 4;
  if (m.b == operand_type::v)
    bits -= 4;
  if (m.c == operand_type::v)
    bits -= 4;
  return bits;
}

// returns an appropriately sized mask to extract an operand of type t from an opcode
int operand_mask(const opmeta& m, operand_type t) {
  switch (t) {
  case operand_type::v:
    // variables are always 4 bits
    return 0xF;
  case operand_type::k: {
    switch (freebits(m)) {
    case 12:
      return 0xFFF;
    case 8:
      return 0xFF;
    case 4:
      return 0xF;
    default:
      return 0;
    }
  }
  case operand_type::addr:
    // addresses are always 12 bits
    return 0xFFF;
  default:
    return 0;
  }
}

bool has_param(const opmeta& m, int operand_num) {
  return m.parameter(operand_num) != operand_type::none;
}

uint16_t extract_param(const opmeta& m, uint16_t opcode, int operand_num) {
  assert(operand_num >= 0 && operand_num < 3);

  int mask = 0, shift = 0;

  switch (operand_num) {
  case 0:
    mask = operand_mask(m, m.a);
    shift = m.ashift;
    break;
  case 1:
    mask = operand_mask(m, m.b);
    shift = m.bshift;
    break;
  case 2:
    mask = operand_mask(m, m.c);
    shift = m.cshift;
    break;
  }
  
  return (opcode & (mask << shift)) >> shift;
}

const char* to_string(operand_type t) {
  switch (t) {
  case operand_type::i:
    return "i";
  case operand_type::v:
    return "v";
  case operand_type::k:
    return "k";
  case operand_type::addr:
    return "addr";
  case operand_type::dt:
    return "dt";
  case operand_type::st:
    return "st";
  default:
    return "unknown";
  }
}

std::string param_string(const opmeta& m, uint16_t opcode, int operand_num) {
  switch (m.parameter(operand_num)) {
  case operand_type::i:
    return "i";
  case operand_type::st:
    return "st";
  case operand_type::dt:
    return "dt";
  case operand_type::v: {
    char buf[4] = {0};
    snprintf(buf, sizeof(buf), "v%X", extract_param(m, opcode, operand_num));
    return buf;
  }
  case operand_type::addr:
  case operand_type::k: {
    char buf[16] = {0};
    snprintf(buf, sizeof(buf), "0x%x", extract_param(m, opcode, operand_num));
    return buf;
  }
  default:
    return "[unknown]";
  }
}

uint16_t strip_operands(uint16_t opcode) { return opcode & strip_mask(opcode); }

const opmeta* opmetatable::find_opcode(uint16_t opcode) const {
  opcode = strip_operands(opcode);
  auto it = std::find_if(std::begin(_meta), std::end(_meta), [=](const auto& m) {
    return opcode == m.opcode;
  });
  if (it != std::end(_meta)) {
    return &*it;
  } else {
    return nullptr;
  }
}