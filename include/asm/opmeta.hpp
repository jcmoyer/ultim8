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

#ifndef ASM_OPMETA_HPP
#define ASM_OPMETA_HPP

#include <cstdint>
#include <string_view>
#include <vector>
#include <string>
#include <tuple>
#include <cassert>

enum class operand_type {
  // i pointer register
  i,
  // variable register
  v,
  // constant
  k,
  // label
  addr,
  // delay timer register
  dt,
  // sound timer register
  st,
  // operand unused
  none
};

const char* to_string(operand_type t);

struct opmeta {
  // assembly mnemonic, which may map to one or more opcodes
  std::string_view mnemonic;

  // types of parameters acceptable for this opcode
  operand_type a = operand_type::none;
  operand_type b = operand_type::none;
  operand_type c = operand_type::none;

  // the instruction's representation if a, b, and c match the usage pattern
  uint16_t opcode = 0;

  // when substitution into the opcode template occurs this is the amount to shift
  // each parameter by
  uint8_t ashift = 0;
  uint8_t bshift = 0;
  uint8_t cshift = 0;

  int parameter_count() const {
    int i = 0;
    if (a != operand_type::none) ++i;
    if (b != operand_type::none) ++i;
    if (c != operand_type::none) ++i;
    return i;
  }

  operand_type parameter(int operand_num) const {
    assert(operand_num >= 0 && operand_num < 3);
    switch (operand_num) {
    case 0:
      return a;
    case 1:
      return b;
    case 2:
      return c;
    default:
      return operand_type::none;
    }
  }
};

inline bool operator<(const opmeta& x, const opmeta& y) {
  return std::tie(x.mnemonic, x.a, x.b, x.c) < std::tie(y.mnemonic, y.a, y.b, y.c);
}

inline bool operator==(const opmeta& x, const opmeta& y) {
  return std::tie(x.mnemonic, x.a, x.b, x.c) == std::tie(y.mnemonic, y.a, y.b, y.c);
}

inline uint16_t generate_op(opmeta m, int a, int b, int c) {
  uint16_t instr = m.opcode;
  instr |= (a << m.ashift);
  instr |= (b << m.bshift);
  instr |= (c << m.cshift);
  return instr;
}

class opmetatable {
public:
  opmetatable();

  const opmeta* find_signature(const opmeta& sig) const;
  const opmeta* find_opcode(uint16_t opcode) const;
  bool is_mnemonic(std::string_view str) const;

  std::vector<const opmeta*> get_signatures(std::string_view mnemonic) const;

private:
  std::vector<opmeta> _meta;
};

bool has_param(const opmeta& m, int operand_num);
std::string param_string(const opmeta& m, uint16_t opcode, int operand_num);

#endif