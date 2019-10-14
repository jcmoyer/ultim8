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

#ifndef EMU_INSTRUCTION_HPP
#define EMU_INSTRUCTION_HPP

#include <cstdint>
#include "common/eswap.hpp"

struct decoded_instruction {
  uint8_t op, a, b, c;
  uint8_t bc;
  uint16_t abc;
  uint16_t opcode;
};

inline constexpr decoded_instruction decode(uint16_t instr) {
  decoded_instruction i{};

  // instructions are big-endian 16-bit integers
  // first, swap to little-endian
  instr = eswap(instr);

  i.opcode = instr;

  i.op = (instr & 0xF000) >> 12;
  i.a = (instr & 0x0F00) >> 8;
  i.b = (instr & 0x00F0) >> 4;
  i.c = instr & 0x000F;

  i.abc = instr & 0xFFF;
  i.bc = instr & 0xFF;

  return i;
}

#endif