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

#ifndef ASM_PARSER_HPP
#define ASM_PARSER_HPP

#include "asm/opmeta.hpp"
#include "asm/error.hpp"
#include <cstdint>
#include <string_view>
#include <vector>

struct token;
class lexer;

// intermediate representation of a chip-8 instruction
struct ir_instruction {
  // describes how this instruction behaves
  const opmeta* m = nullptr;

  // operands to substitute into the opmeta template
  int a, b, c;

  // if one of the opmeta parameters is addr, label_ref is the name of the label
  std::string_view label_ref;

  // only used for data metainstruction
  std::vector<uint8_t> parameters;

  // used by labels to fixup references
  std::size_t address;

  std::size_t size() const {
    if (is_data()) {
      return parameters.size();
    } else {
      return 2;
    }
  }

  bool is_data() const { return parameters.size(); }
};

struct label {
  std::string_view name;
  std::size_t instr_index;
};

class parser {
public:
  parser(lexer& lex);

  std::vector<ir_instruction> parse_instructions();

private:
  const token& current();
  const token& next();
  void add_instruction(ir_instruction i);
  void parse_top_level();
  void parse_label();
  void parse_data();
  void parse_instruction();
  void resolve_labels();
  void error(const char* msg, const token& t);
  void error(const char* msg, const token& t, std::string help);

  std::vector<ir_instruction> _instructions;
  std::vector<label> _labels;

  lexer& _lex;

  std::size_t _address = 0x200;

  opmetatable _omtbl;
};

#include <iosfwd>

uint16_t encode_instruction(const ir_instruction& instr);
void write_instruction(std::ostream& output, const ir_instruction& instr);
void write_instruction(std::vector<uint8_t>& output, const ir_instruction& instr);

#endif