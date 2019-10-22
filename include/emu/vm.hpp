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

#ifndef EMU_VM_HPP
#define EMU_VM_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <cassert>
#include <random>

#include "emu/framebuffer.hpp"
#include "emu/input.hpp"

// instruction dispatching types
class variable {
public:
  constexpr variable(uint8_t i) : _i{i} {}
  constexpr operator uint8_t() const { return _i; }

private:
  uint8_t _i;
};

struct dtreg {};
struct streg {};
struct ireg {};

// state of cpu; calling step() will set this and allows host programs to
// handle various failure cases
enum class cpu_status {
  ok,
  // instruction not recognized
  invalid_instruction,
  // call stack is empty
  no_return
};

inline constexpr const char* cpu_status_str(cpu_status s) {
  switch (s) {
  case cpu_status::ok:
    return "ok";
  case cpu_status::invalid_instruction:
    return "invalid_instruction";
  case cpu_status::no_return:
    return "no_return";
  default:
    return "unknown";
  }
}

class chip8vm {
public:
  // XO sized roms fit in 64k of ram, however we add some additional padding to
  // make sure roms can never read/write outside of memory
  static constexpr std::size_t MEMORY_SIZE = 0x11000;
  static constexpr std::size_t VARIABLE_COUNT = 16;
  static constexpr std::size_t PROGRAM_START = 0x200;
  static constexpr std::size_t PROGRAM_MAX_SIZE = MEMORY_SIZE - PROGRAM_START;
  static constexpr std::size_t FONT_START = 0;
  static constexpr std::size_t FONT_GLYPH_SIZE = 5;
  static constexpr std::size_t BIGFONT_START = 0x100;
  static constexpr std::size_t BIGFONT_GLYPH_SIZE = 10;

  std::array<uint8_t, MEMORY_SIZE> memory{0};
  std::array<uint8_t, VARIABLE_COUNT> variables{0};
  std::array<uint8_t, VARIABLE_COUNT> rpl{0};
  std::vector<std::size_t> callstack;
  framebuffer framebuf{64, 32};
  input_state inp;
  cpu_status status = cpu_status::ok;
  uint16_t pc{PROGRAM_START};
  uint16_t i{0};
  uint8_t dt{0};
  uint8_t st{0};

  chip8vm();

  // reinitializes all state
  void reset() {
    memory.fill(0);
    copy_font_glyphs();
    variables.fill(0);
    rpl.fill(0);
    callstack.clear();
    lores();
    inp.clear();
    status = cpu_status::ok;
    pc = PROGRAM_START;
    i = 0;
    dt = 0;
    st = 0;
  }

  void dec_timers() {
    if (dt)
      --dt;
    if (st)
      --st;
  }

  void step();

private:
  // carry flag, borrow flag, collision flag
  void vf(bool value) { variables[0xF] = value; }

  // note: it's possible this read will be unaligned
  // should probably profile it at some point, it seems to be common with modern roms
  uint16_t fetch() const { return *reinterpret_cast<const uint16_t*>(&memory[pc]); }

  void copy_font_glyphs();

  void draw_sprite(int x, int y, int h);
  void draw_small_sprite(int x, int y, int h);
  void draw_big_sprite(int x, int y);

  void init_step_context() {
    step_context = chip8_step_context(pc);
    step_context.set_pending_pc(pc + sizeof(uint16_t));
  }

  void apply_step_context() { pc = step_context.get_pending_pc(); }

  // these functions are named after instruction mnemonics (see asm/opmeta.cpp)
  void cls();
  void ret();
  void hires();
  void lores();
  void exit();
  void jmp(uint16_t);
  void call(uint16_t);
  void skeq(variable, uint8_t);
  void skne(variable, uint8_t);
  void skeq(variable, variable);
  void ld(variable, uint8_t);
  void add(variable, uint8_t);
  void ld(variable, variable);
  void or_(variable, variable);
  void and_(variable, variable);
  void xor_(variable, variable);
  void add(variable, variable);
  void sub(variable, variable);
  void shr(variable, variable);
  void subn(variable, variable);
  void shl(variable, variable);
  void skne(variable, variable);
  void ld(ireg, uint16_t);
  void jmp0(uint16_t);
  void rand(variable, uint8_t);
  void disp(variable, variable, uint8_t);
  void skp(variable);
  void sknp(variable);
  void audio();
  void ld(variable, dtreg);
  void input(variable);
  void ld(dtreg, variable);
  void ld(streg, variable);
  void add(ireg, variable);
  void glyph(variable);
  void bglyph(variable);
  void bcd(variable);
  void store(variable);
  void load(variable);
  void storeflags(variable);
  void loadflags(variable);

  // dispatch to 0xxx
  void sys(uint16_t abc);
  // dispatch to 8xxx
  void arith(variable a, variable b, uint8_t c);
  // dispatch to Exxx
  void inputctl(variable a, uint8_t bc);
  // dispatch to Fxxx
  void misc(uint8_t a, uint8_t bc);

  void bad_instr();

  // enables buffered pc operations so that during an instruction handler,
  // pc points to the instruction that the handler is executing for
  class chip8_step_context {
  public:
    chip8_step_context() : _oldpc{}, _newpc{} {}
    chip8_step_context(uint16_t pc) : _oldpc{pc}, _newpc(pc + sizeof(uint16_t)) {}

    void revert_pc() { _newpc = _oldpc; }
    void skip_next_instr() { _newpc += sizeof(uint16_t); }
    uint16_t current_instruction_pc() const { return _oldpc; }
    uint16_t next_instruction_pc() const { return _oldpc + sizeof(uint16_t); }
    void set_pending_pc(uint16_t new_pc) { _newpc = new_pc; }
    uint16_t get_pending_pc() const { return _newpc; }

  private:
    uint16_t _oldpc;
    uint16_t _newpc;
  };

private:
  std::random_device rd;
  std::mt19937 rng;
  std::uniform_int_distribution<> byte_dist;
  chip8_step_context step_context;
};

#endif
