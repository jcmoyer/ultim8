#include <catch.hpp>
#include <memory>
#include "asm/opmeta.hpp"
#include "emu/vm.hpp"
#include "common/eswap.hpp"

// loads a single opcode and executes it
void execute(chip8vm& state, uint16_t opcode) {
  uint16_t* write = reinterpret_cast<uint16_t*>(state.memory.data() + chip8vm::PROGRAM_START);
  *write = eswap(opcode);
  state.step();
}

// executes a sequence of instructions until the program counter stops changing
void execute_to_stable(chip8vm& state, const std::initializer_list<uint16_t>& opcodes) {
  uint16_t* write = reinterpret_cast<uint16_t*>(state.memory.data() + chip8vm::PROGRAM_START);
  for (const auto& opcode : opcodes)
    *write++ = eswap(opcode);

  uint16_t last_pc;
  do {
    last_pc = state.pc;
    state.step();
  } while (last_pc != state.pc);
}

TEST_CASE("single instructions") {
  auto p = std::make_unique<chip8vm>();

  SECTION("0xxx") {
    SECTION("00E0") {
      p->framebuf.toggle(10, 10);
      p->framebuf.toggle(3, 15);
      execute(*p, 0x00E0);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(!p->framebuf.is_on(10, 10));
      REQUIRE(!p->framebuf.is_on(3, 15));
    }
    SECTION("00EE") {
      execute_to_stable(*p,
        {
          0x2204, // 0200: call 0x204
          0x1202, // 0202: jmp  0x202
          0x00EE, // 0204: ret
        });
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->callstack.size() == 0);
      REQUIRE(p->pc == 0x0202);
    }
  }

  SECTION("1xxx") {
    execute(*p, 0x1123);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->pc == 0x123);
  }

  SECTION("2xxx") {
    execute(*p, 0x2200);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->callstack.size() == 1);
    REQUIRE(p->pc == 0x200);
  }

  SECTION("3xxx") {
    SECTION("equal") {
      p->variables[2] = 0x10;
      execute(*p, 0x3210);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x204);
    }
    SECTION("not equal") {
      p->variables[2] = 0x20;
      execute(*p, 0x3210);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x202);
    }
  }

  SECTION("4xxx") {
    SECTION("equal") {
      p->variables[2] = 0x10;
      execute(*p, 0x4210);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x202);
    }
    SECTION("not equal") {
      p->variables[2] = 0x20;
      execute(*p, 0x4210);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x204);
    }
  }

  SECTION("5xxx") {
    SECTION("equal") {
      p->variables[2] = 0x10;
      p->variables[4] = 0x10;
      execute(*p, 0x5240);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x204);
    }
    SECTION("not equal") {
      p->variables[2] = 0x10;
      p->variables[4] = 0x20;
      execute(*p, 0x5240);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x202);
    }
  }

  SECTION("6xxx") {
    execute(*p, 0x60FF);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->variables[0] == 0xFF);
  }

  SECTION("7xxx") {
    p->variables[2] = 10;
    execute(*p, 0x720a);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->variables[2] == 20);
  }

  SECTION("8xxx") {
    SECTION("8xx0") {
      p->variables[1] = 10;
      p->variables[2] = 20;
      execute(*p, 0x8120);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == 20);
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx1") {
      p->variables[1] = 10;
      p->variables[2] = 20;
      execute(*p, 0x8121);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == (10 | 20));
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx2") {
      p->variables[1] = 10;
      p->variables[2] = 20;
      execute(*p, 0x8122);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == (10 & 20));
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx3") {
      p->variables[1] = 10;
      p->variables[2] = 20;
      execute(*p, 0x8123);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == (10 ^ 20));
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx4") {
      p->variables[1] = 10;
      p->variables[2] = 20;
      execute(*p, 0x8124);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == (10 + 20));
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx5") {
      p->variables[1] = 30;
      p->variables[2] = 20;
      execute(*p, 0x8125);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == (30 - 20));
      REQUIRE(p->variables[2] == 20);
    }
    SECTION("8xx6") {
      p->variables[1] = 10;
      p->variables[2] = 40;
      execute(*p, 0x8126);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == 20);
      REQUIRE(p->variables[2] == 40);
    }
    SECTION("8xx7") {
      p->variables[1] = 10;
      p->variables[2] = 40;
      execute(*p, 0x8127);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == 30);
      REQUIRE(p->variables[2] == 40);
    }
    SECTION("8xxE") {
      p->variables[1] = 10;
      p->variables[2] = 40;
      execute(*p, 0x812E);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[1] == 80);
      REQUIRE(p->variables[2] == 40);
    }
  }

  SECTION("9xxx") {
    SECTION("equal") {
      p->variables[2] = 0x10;
      p->variables[4] = 0x10;
      execute(*p, 0x9240);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x202);
    }
    SECTION("not equal") {
      p->variables[2] = 0x10;
      p->variables[4] = 0x20;
      execute(*p, 0x9240);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->pc == 0x204);
    }
  }

  SECTION("Axxx") {
    execute(*p, 0xA124);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->i == 0x124);
  }

  SECTION("Bxxx") {
    execute_to_stable(*p,
      {
        0x6004, // 0200: ld    v0, 4
        0xB200, // 0202: jmp0  0x200
        0x1204  // 0204: jmp   0x204
      });
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE(p->pc == 0x0204);
  }

  SECTION("Cxxx") {
    // TODO: come up with a better test for this
    execute(*p, 0xC001);
    REQUIRE(p->status == cpu_status::ok);
    REQUIRE((p->variables[0] == 0 || p->variables[0] == 1));
  }

  SECTION("Dxxx") {
    p->memory[0x300] = 0b10101011;
    p->memory[0x301] = 0b11111111;
    execute_to_stable(*p,
      {
        0xA300, // 0200: ld    i, 0x300
        0x6002, // 0202: ld    v0, 2
        0xD002, // 0204: disp  v0, v0, 2
        0x1206, // 0206: jmp   0x206
      });
    REQUIRE(p->status == cpu_status::ok);
    // row 1
    REQUIRE(p->framebuf.is_on(2, 2));
    REQUIRE(p->framebuf.is_on(4, 2));
    REQUIRE(p->framebuf.is_on(6, 2));
    REQUIRE(p->framebuf.is_on(8, 2));
    REQUIRE(p->framebuf.is_on(9, 2));
    // row 2
    REQUIRE(p->framebuf.is_on(2, 3));
    REQUIRE(p->framebuf.is_on(3, 3));
    REQUIRE(p->framebuf.is_on(4, 3));
    REQUIRE(p->framebuf.is_on(5, 3));
    REQUIRE(p->framebuf.is_on(6, 3));
    REQUIRE(p->framebuf.is_on(7, 3));
    REQUIRE(p->framebuf.is_on(8, 3));
    REQUIRE(p->framebuf.is_on(9, 3));
  }

  SECTION("Exxx") {
    SECTION("Ex9E") {
      SECTION("pressed") {
        p->inp.set_key_state(HEXKEY_4, true);
        p->variables[0] = 4;
        execute(*p, 0xE09E);
        REQUIRE(p->status == cpu_status::ok);
        REQUIRE(p->pc == 0x204);
      }
      SECTION("not pressed") {
        p->inp.set_key_state(HEXKEY_4, false);
        p->variables[0] = 4;
        execute(*p, 0xE09E);
        REQUIRE(p->status == cpu_status::ok);
        REQUIRE(p->pc == 0x202);
      }
    }
    SECTION("ExA1") {
      SECTION("pressed") {
        p->inp.set_key_state(HEXKEY_4, true);
        p->variables[0] = 4;
        execute(*p, 0xE0A1);
        REQUIRE(p->status == cpu_status::ok);
        REQUIRE(p->pc == 0x202);
      }
      SECTION("not pressed") {
        p->inp.set_key_state(HEXKEY_4, false);
        p->variables[0] = 4;
        execute(*p, 0xE0A1);
        REQUIRE(p->status == cpu_status::ok);
        REQUIRE(p->pc == 0x204);
      }
    }
  }

  SECTION("Fxxx") {
    SECTION("Fx07") {
      p->dt = 60;
      execute(*p, 0xF007);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[0] == 60);
    }
    SECTION("Fx0A") {
      p->inp.set_key_state(HEXKEY_4, true);
      execute(*p, 0xF00A);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[0] == 4);
    }
    SECTION("Fx15") {
      p->variables[0] = 60;
      execute(*p, 0xF015);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->dt == 60);
    }
    SECTION("Fx18") {
      p->variables[0] = 60;
      execute(*p, 0xF018);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->st == 60);
    }
    SECTION("Fx1E") {
      p->i = 200;
      p->variables[4] = 60;
      execute(*p, 0xF41E);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->i == 260);
    }
    SECTION("Fx29") {
      p->i = 0xFFFF;
      p->variables[4] = 1;
      execute(*p, 0xF429);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->i < 0x200);
    }
    SECTION("Fx33") {
      p->variables[4] = 123;
      p->i = 0x300;
      execute(*p, 0xF433);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->memory[0x300] == 1);
      REQUIRE(p->memory[0x301] == 2);
      REQUIRE(p->memory[0x302] == 3);
    }
    SECTION("Fx55") {
      p->variables[0] = 12;
      p->variables[1] = 34;
      p->variables[2] = 56;
      p->variables[3] = 78;
      p->i = 0x300;
      execute(*p, 0xF355);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->memory[0x300] == 12);
      REQUIRE(p->memory[0x301] == 34);
      REQUIRE(p->memory[0x302] == 56);
      REQUIRE(p->memory[0x303] == 78);
    }
    SECTION("Fx65") {
      p->i = 0x300;
      p->memory[0x300] = 12;
      p->memory[0x301] = 34;
      p->memory[0x302] = 56;
      p->memory[0x303] = 78;      
      execute(*p, 0xF365);
      REQUIRE(p->status == cpu_status::ok);
      REQUIRE(p->variables[0] == 12);
      REQUIRE(p->variables[1] == 34);
      REQUIRE(p->variables[2] == 56);
      REQUIRE(p->variables[3] == 78);
    }
  }
}
