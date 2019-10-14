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

#include "emu/vm.hpp"
#include "emu/instruction.hpp"
#include <cstring>

// clang-format off
// 4x8 font glyphs
uint8_t font_data[] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, 
  0x20, 0x60, 0x20, 0x20, 0x70, 
  0xF0, 0x10, 0xF0, 0x80, 0xF0, 
  0xF0, 0x10, 0xF0, 0x10, 0xF0,
  0x90, 0x90, 0xF0, 0x10, 0x10, 
  0xF0, 0x80, 0xF0, 0x10, 0xF0,
  0xF0, 0x80, 0xF0, 0x90, 0xF0, 
  0xF0, 0x10, 0x20, 0x40, 0x40, 
  0xF0, 0x90, 0xF0, 0x90, 0xF0, 
  0xF0, 0x90, 0xF0, 0x10, 0xF0, 
  0xF0, 0x90, 0xF0, 0x90, 0x90, 
  0xE0, 0x90, 0xE0, 0x90, 0xE0, 
  0xF0, 0x80, 0x80, 0x80, 0xF0, 
  0xE0, 0x90, 0x90, 0x90, 0xE0, 
  0xF0, 0x80, 0xF0, 0x80, 0xF0,
  0xF0, 0x80, 0xF0, 0x80, 0x80,
};

// superchip, 8x10 font glyphs
uint8_t bigfont_data[] = {
  0x3C, 0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x3C,
  0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C,
  0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF,
  0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C,
  0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06,
  0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C,
  0x3E, 0x7C, 0xC0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C,
  0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60,
  0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C,
  0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C,
  // TODO: original superchip only implements 0..9
  // maybe add the other glyphs later
};
// clang-format on

chip8vm::chip8vm() : rng(rd()), byte_dist(0, 255) {
  copy_font_glyphs();
}

void chip8vm::step() {
  decoded_instruction instr = decode(fetch());

  if (status != cpu_status::ok) {
    return;
  }

  init_step_context();

  // clang-format off
  const uint8_t  op  = instr.op;
  const uint8_t  a   = instr.a;
  const uint8_t  b   = instr.b;
  const uint8_t  c   = instr.c;
  const uint16_t abc = instr.abc;
  const uint8_t  bc  = instr.bc;

  switch (op) {
  case 0x0: sys(abc);                           break;
  case 0x1: jmp(abc);                           break;
  case 0x2: call(abc);                          break;
  case 0x3: skeq(variable{a}, bc);              break;
  case 0x4: skne(variable{a}, bc);              break;
  case 0x5: skeq(variable{a}, variable{b});     break;
  case 0x6: ld(variable{a}, bc);                break;
  case 0x7: add(variable{a}, bc);               break;
  case 0x8: arith(variable{a}, variable{b}, c); break;
  case 0x9: skne(variable{a}, variable{b});     break;
  case 0xA: ld(ireg{}, abc);                    break;
  case 0xB: jmp0(abc);                          break;
  case 0xC: rand(variable{a}, bc);              break;
  case 0xD: disp(variable{a}, variable{b}, c);  break;
  case 0xE: inputctl(variable{a}, bc);          break;
  case 0xF: misc(a, bc);                        break;
  default: bad_instr();                         break;
  }
  // clang-format on

  if (status == cpu_status::ok) {
    inp.clear_last_key();
  } else {
    // if something went wrong we want to preserve the pc for debug purposes
    step_context.revert_pc();
  }

  apply_step_context();
}

void chip8vm::copy_font_glyphs() {
  memcpy(memory.data() + FONT_START, font_data, sizeof(font_data));
  memcpy(memory.data() + BIGFONT_START, bigfont_data, sizeof(bigfont_data));
}

void chip8vm::draw_sprite(int x, int y, int height) {
  vf(0);
  if (height)
    draw_small_sprite(x, y, height);
  else
    draw_big_sprite(x, y);
}

void chip8vm::draw_small_sprite(int x, int y, int height) {
  for (int yo = 0; yo < height; ++yo) {
    uint8_t row = memory[i + yo];
    for (uint8_t mask = 0b10000000, xo = 0; mask; mask >>= 1, ++xo) {
      if (row & mask) {
        if (!framebuf.toggle(x + xo, y + yo)) {
          vf(1);
        }
      }
    }
  }
}

void chip8vm::draw_big_sprite(int x, int y) {
  for (int yo = 0; yo < 16; ++yo) {
    uint16_t row = (memory[i + 2 * yo] << 8) | (memory[i + 2 * yo + 1] << 0);
    for (uint16_t mask = 0b10000000'00000000, xo = 0; mask; mask >>= 1, ++xo) {
      if (row & mask) {
        if (!framebuf.toggle(x + xo, y + yo)) {
          vf(1);
        }
      }
    }
  }
}

// clang-format off
inline void chip8vm::misc(uint8_t a, uint8_t bc) {
  switch (bc) {
  case 0x02: audio();                  break;
  case 0x07: ld(variable{a}, dtreg{}); break;
  case 0x0A: input(variable{a});       break;
  case 0x15: ld(dtreg{}, variable{a}); break;
  case 0x18: ld(streg{}, variable{a}); break;
  case 0x1E: add(ireg{}, variable{a}); break;
  case 0x29: glyph(variable{a});       break;
  case 0x30: bglyph(variable{a});      break;
  case 0x33: bcd(variable{a});         break;
  case 0x55: store(variable{a});       break;
  case 0x65: load(variable{a});        break;
  case 0x75: storeflags(variable{a});  break;
  case 0x85: loadflags(variable{a});   break;
  default: bad_instr();                break;
  }
}

inline void chip8vm::sys(uint16_t abc) {
  switch (abc) {
  case 0x0E0: cls();    break;
  case 0x0EE: ret();    break;
  case 0x0FD: exit();   break;
  case 0x0FE: lores();  break;
  case 0x0FF: hires();  break;
  default: bad_instr(); break;
  }
}

inline void chip8vm::arith(variable a, variable b, uint8_t c) {
  switch (c) {
  case 0x0: ld  (a, b); break;
  case 0x1: or_ (a, b); break;
  case 0x2: and_(a, b); break;
  case 0x3: xor_(a, b); break;
  case 0x4: add (a, b); break;
  case 0x5: sub (a, b); break;
  case 0x6: shr (a, b); break;
  case 0x7: subn(a, b); break;
  case 0xE: shl (a, b); break;
  default: bad_instr(); break;
  }
}

inline void chip8vm::inputctl(variable a, uint8_t bc) {
  switch (bc) {
  case 0x9E: skp(a);    break;
  case 0xA1: sknp(a);   break;
  default: bad_instr(); break;
  }
}
// clang-format on

// 0x00E0
inline void chip8vm::cls() {
  framebuf.clear();
}

// 0x00EE
inline void chip8vm::ret() {
  if (callstack.size()) {
    step_context.set_pending_pc(static_cast<uint16_t>(callstack.back()));
    callstack.pop_back();
  } else {
    status = cpu_status::no_return;
  }
}

// 0x00FF
inline void chip8vm::hires() {
  framebuf = framebuffer{128, 64};
}

// 0x00FE
inline void chip8vm::lores() {
  framebuf = framebuffer{64, 32};
}

// 0x00FD
inline void chip8vm::exit() {
  status = cpu_status::invalid_instruction;
}

// 0x1000
inline void chip8vm::jmp(uint16_t abc) {
  step_context.set_pending_pc(abc);
}

// 0x2000
inline void chip8vm::call(uint16_t abc) {
  callstack.push_back(step_context.get_pending_pc());
  step_context.set_pending_pc(abc);
}

// 0x3000
inline void chip8vm::skeq(variable a, uint8_t bc) {
  if (variables[a] == bc) {
    step_context.skip_next_instr();
  }
}

// 0x4000
inline void chip8vm::skne(variable a, uint8_t bc) {
  if (variables[a] != bc) {
    step_context.skip_next_instr();
  }
}

// 0x5000
inline void chip8vm::skeq(variable a, variable b) {
  if (variables[a] == variables[b]) {
    step_context.skip_next_instr();
  }
}

// 0x6000
inline void chip8vm::ld(variable a, uint8_t bc) {
  variables[a] = bc;
}

// 0x7000
inline void chip8vm::add(variable a, uint8_t bc) {
  variables[a] += bc;
}

// 0x8000
inline void chip8vm::ld(variable a, variable b) {
  variables[a] = variables[b];
}

// 0x8001
inline void chip8vm::or_(variable a, variable b) {
  variables[a] |= variables[b];
}

// 0x8002
inline void chip8vm::and_(variable a, variable b) {
  variables[a] &= variables[b];
}

// 0x8003
inline void chip8vm::xor_(variable a, variable b) {
  variables[a] ^= variables[b];
}

// 0x8004
inline void chip8vm::add(variable a, variable b) {
  // !!! SEQUENCING IS VERY, VERY IMPORTANT IN OPERATIONS THAT MODIFY VF !!!
  //
  // If VF is modified, then VF is read from immediately afterwards (because VF was an
  // operand) the result will be different than if VF were written after performing that
  // operation.
  //
  // I believe the correct implementation is that the result should be computed first using
  // the values currently stored in the registers. Then, update vF and the destination
  // register using that result.
  //
  // Most games do not care about sequencing but octopeg is an example of a rom that does.
  //
  // Here is an example of an incorrect implementation:
  //
  // vF = v[A] + v[B] > 0xFF      <-- vF modified
  // v[A] = v[A] + v[B]           <-- if v[A] or v[B] are vF, this answer changes
  uint16_t result = variables[a] + variables[b];
  vf(result > 0xFF);
  variables[a] = static_cast<uint8_t>(result);
}

// 0x8005
inline void chip8vm::sub(variable a, variable b) {
  // COSMAC VIP manual states:
  //
  //   VF = 01 if VX >= VY
  //
  // There are other references that state VF = 01 if VX > VY, which is incorrect.
  uint8_t result = variables[a] - variables[b];
  vf(variables[a] >= variables[b]);
  variables[a] = result;
}

// 0x8006
inline void chip8vm::shr(variable a, variable b) {
  uint8_t result = variables[b] >> 1;
  vf(variables[b] & 0b00000001);
  variables[a] = result;
}

// 0x8007
inline void chip8vm::subn(variable a, variable b) {
  uint8_t result = variables[b] - variables[a];
  vf(variables[b] >= variables[a]);
  variables[a] = result;
}

// 0x800E
inline void chip8vm::shl(variable a, variable b) {
  uint8_t result = variables[b] << 1;
  vf(variables[b] & 0b10000000);
  variables[a] = result;
}

// 0x9000
inline void chip8vm::skne(variable a, variable b) {
  if (variables[a] != variables[b]) {
    step_context.skip_next_instr();
  }
}

// 0xA000
inline void chip8vm::ld(ireg, uint16_t abc) {
  i = abc;
}

// 0xB000
inline void chip8vm::jmp0(uint16_t abc) {
  step_context.set_pending_pc(static_cast<size_t>(variables[0]) + static_cast<size_t>(abc));
}

// 0xC000
inline void chip8vm::rand(variable a, uint8_t bc) {
  variables[a] = byte_dist(rng) & bc;
}

// 0xD000
inline void chip8vm::disp(variable a, variable b, uint8_t c) {
  int x = variables[a];
  int y = variables[b];
  int h = c;
  draw_sprite(x, y, h);
}

// 0xE09E
inline void chip8vm::skp(variable a) {
  if (inp.is_pressed(static_cast<chip8_key>(variables[a] & 0xF))) {
    step_context.skip_next_instr();
  }
}

// 0xE0A1
inline void chip8vm::sknp(variable a) {
  if (!inp.is_pressed(static_cast<chip8_key>(variables[a] & 0xF))) {
    step_context.skip_next_instr();
  }
}

// 0xF002
inline void chip8vm::audio() {
  // XO extension; not implemented
}

// 0xF007
inline void chip8vm::ld(variable a, dtreg) {
  variables[a] = dt;
}

// 0xF00A
inline void chip8vm::input(variable a) {
  // blocking operation
  if (inp.has_last_key()) {
    variables[a] = inp.last_key;
  } else {
    step_context.revert_pc();
  }
}

// 0xF015
inline void chip8vm::ld(dtreg, variable a) {
  dt = variables[a];
}

// 0xF018
inline void chip8vm::ld(streg, variable a) {
  st = variables[a];
}

// 0xF01E
inline void chip8vm::add(ireg, variable a) {
  i += variables[a];
}

// 0xF029
inline void chip8vm::glyph(variable a) {
  i = FONT_START + FONT_GLYPH_SIZE * static_cast<size_t>(variables[a] & 0xF);
}

// 0xF030 TODO: opmeta
inline void chip8vm::bglyph(variable a) {
  i = BIGFONT_START + BIGFONT_GLYPH_SIZE * static_cast<size_t>(variables[a] & 0xF);
}

// 0xF033
inline void chip8vm::bcd(variable a) {
  uint8_t d = variables[a];
  memory[static_cast<size_t>(i) + 2] = d % 10;
  d /= 10;
  memory[static_cast<size_t>(i) + 1] = d % 10;
  d /= 10;
  memory[i] = d % 10;
}

// 0xF055
inline void chip8vm::store(variable a) {
  for (int i = 0; i <= a; ++i) {
    memory[this->i + i] = variables[i];
  }
  i = i + a + 1;
}

// 0xF065
inline void chip8vm::load(variable a) {
  for (int i = 0; i <= a; ++i) {
    variables[i] = memory[this->i + i];
  }
  i = i + a + 1;
}

// 0xF075
inline void chip8vm::storeflags(variable a) {
  for (int i = 0; i <= a; ++i) {
    rpl[i] = variables[i];
  }
}

// 0xF085
inline void chip8vm::loadflags(variable a) {
  for (int i = 0; i <= a; ++i) {
    variables[i] = rpl[i];
  }
}

inline void chip8vm::bad_instr() {
  status = cpu_status::invalid_instruction;
}
