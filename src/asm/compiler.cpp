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

#include "asm/compiler.hpp"
#include "asm/lexer.hpp"
#include "asm/parser.hpp"

std::vector<uint8_t> compile(const char* source) {
  std::vector<uint8_t> bytes;
  lexer l(source);
  parser p(l);
  for (const ir_instruction& instr : p.parse_instructions()) {
    write_instruction(bytes, instr);
  }
  return bytes;
}
