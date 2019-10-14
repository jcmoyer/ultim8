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

#include "asm/parser.hpp"
#include "asm/lexer.hpp"
#include <algorithm>
#include <cassert>

constexpr operand_type token_to_operand_type(token_type t) {
  switch (t) {
  case token_type::variable:
    return operand_type::v;
  case token_type::number:
    return operand_type::k;
  case token_type::text:
    return operand_type::addr;
  case token_type::i:
    return operand_type::i;
  case token_type::dt:
    return operand_type::dt;
  case token_type::st:
    return operand_type::st;
  default:
    assert(false);
    return operand_type::none;
  }
}

parser::parser(lexer& lex) : _lex{ lex } {
  _lex.next();
}

std::vector<ir_instruction> parser::parse_instructions() {
  while (current().type != token_type::eos) {
    parse_top_level();
  }

  resolve_labels();

  return std::move(_instructions);
}

void parser::resolve_labels() {
  for (auto& instr : _instructions) {
    if (instr.label_ref.size()) {
      auto it = std::find_if(
        _labels.begin(), _labels.end(), [&](const auto& l) { return l.name == instr.label_ref; });
      if (it != _labels.end()) {
        if (instr.m->a == operand_type::addr)
          instr.a = static_cast<int>(_instructions[it->instr_index].address);
        if (instr.m->b == operand_type::addr)
          instr.b = static_cast<int>(_instructions[it->instr_index].address);
        if (instr.m->c == operand_type::addr)
          instr.c = static_cast<int>(_instructions[it->instr_index].address);
      }
    }
  }
}

void parser::parse_data() {
  _lex.next();

  ir_instruction data_pinstr;

  while (next().type == token_type::comma) {
    if (current().type != token_type::number)
      error("expected number", current());
    data_pinstr.parameters.push_back(current().numeric_value);
    _lex.next();
    _lex.next();
  }
  if (current().type != token_type::number)
    error("expected number", current());
  data_pinstr.parameters.push_back(current().numeric_value);
  _lex.next();
  add_instruction(data_pinstr);
}

void parser::parse_instruction() {
  token mnemonic_tok = current();
  auto instr_name = mnemonic_tok.span;

  // first, see if this instruction has a version that doesn't take parameters
  {
    opmeta m;
    m.mnemonic = instr_name;
    m.a = operand_type::none;
    m.b = operand_type::none;
    m.c = operand_type::none;
    if (const auto* it = _omtbl.find_signature(m)) {
      add_instruction(ir_instruction{it, 0, 0, 0});
      _lex.next();
      return;
    }
  }

  // if it doesn't, gather the parameters and try to infer the correct version
  _lex.next();
  std::vector<token> parameters;
  while (next().type == token_type::comma) {
    if (current().type == token_type::eos)
      error("unexpected end of file", current());
    parameters.push_back(current());
    _lex.next();
    _lex.next();
  }
  if (current().type == token_type::eos)
    error("unexpected end of file", current());
  parameters.push_back(current());
  _lex.next();

  if (parameters.size() > 3) {
    error("too many operands", mnemonic_tok);
  }

  // build an opmeta from the parameters to perform a lookup
  opmeta om;
  om.mnemonic = instr_name;
  if (parameters.size() > 0)
    om.a = token_to_operand_type(parameters[0].type);
  if (parameters.size() > 1)
    om.b = token_to_operand_type(parameters[1].type);
  if (parameters.size() > 2)
    om.c = token_to_operand_type(parameters[2].type);

  if (const opmeta* m = _omtbl.find_signature(om); m) {
    int a = om.parameter_count() > 0 ? parameters[0].numeric_value : 0;
    int b = om.parameter_count() > 1 ? parameters[1].numeric_value : 0;
    int c = om.parameter_count() > 2 ? parameters[2].numeric_value : 0;

    ir_instruction instr{m, a, b, c};

    if (om.a == operand_type::addr) {
      instr.label_ref = parameters[0].span;
    }
    if (om.b == operand_type::addr) {
      instr.label_ref = parameters[1].span;
    }

    add_instruction(instr);
  } else {
    std::string help = "instruction forms:\n\n";
    for (const auto& m : _omtbl.get_signatures(instr_name)) {
      help += "  ";
      help += std::string(m->mnemonic) + " ";
      for (int i = 0; i < m->parameter_count(); ++i) {
        help += to_string(m->parameter(i));
        help += ", ";
      }
      help.pop_back();
      help.pop_back();
      help += "\n";
    }
    error("incorrect instruction usage", mnemonic_tok, help);
  }
}

void parser::parse_top_level() {
  if (current().type == token_type::text) {
    if (next().type == token_type::colon) {
      parse_label();
    } else if (current().span == "data") {
      parse_data();
    } else {
      error("expected ':' after label", _lex.current());
    }
  } else if (current().type == token_type::mnemonic) {
    parse_instruction();
  } else {
    error("expected label, data, or mnemonic", _lex.current());
  }
}

void parser::parse_label() {
  _labels.push_back({current().span, _instructions.size()});
  _lex.next();
  _lex.next();
}

const token& parser::current() {
  return _lex.current();
}

const token& parser::next() {
  return _lex.lookahead();
}

void parser::add_instruction(ir_instruction i) {
  _instructions.push_back(std::move(i));
  _instructions.back().address = _address;
  _address += _instructions.back().size();
}

void parser::error(const char* msg, const token& t) {
  throw syntax_error(msg, t.location.line, t.location.pos, std::string(t.span));
}

void parser::error(const char* msg, const token& t, std::string help) {
  throw syntax_error(msg, t.location.line, t.location.pos, std::string(t.span), std::move(help));
}

#include "common/eswap.hpp"
#include <iostream>

uint16_t encode_instruction(const ir_instruction& instr) {
  return generate_op(*instr.m, instr.a, instr.b, instr.c);
}

void write_instruction(std::ostream& output, const ir_instruction& instr) {
  if (!instr.is_data()) {
    uint16_t bytes = eswap(encode_instruction(instr));
    output.write(reinterpret_cast<const char*>(&bytes), sizeof(uint16_t));
  } else {
    output.write(reinterpret_cast<const char*>(instr.parameters.data()), instr.parameters.size());
  }
}

void write_instruction(std::vector<uint8_t>& output, const ir_instruction& instr) {
  if (!instr.is_data()) {
    uint16_t bytes = encode_instruction(instr);
    output.push_back((bytes & 0xFF00) >> 8);
    output.push_back((bytes & 0x00FF) >> 0);
  } else {
    output.insert(output.end(), instr.parameters.begin(), instr.parameters.end());
  }
}