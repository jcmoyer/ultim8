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

#ifndef ASM_LEXER_HPP
#define ASM_LEXER_HPP

#include <string>
#include <string_view>
#include "asm/opmeta.hpp"
#include "asm/error.hpp"

enum class token_type {
  text,
  mnemonic,
  number,
  colon,
  comma,
  eos,
  variable,
  i,
  dt,
  st,
  uninit
};

struct source_location {
  int line = 0, pos = 0;
};

struct token {
  token_type type;
  // view into original source string
  std::string_view span;
  int numeric_value = 0;
  source_location location;
};

class lexer {
public:
  lexer(std::string source);

  void next();

  const token& current() const { return _cur; }
  const token& lookahead() {
    if (_lookahead.type == token_type::uninit) {
      _lookahead = lex_one();
    }
    return _lookahead;
  }

  int line() const { return _line; }
  int pos() const { return _pos; }

private:
  // reads the next character from the string
  char read_one();
  token lex_one();
  void new_line();
  token read_number();
  token read_name();
  void read_comment();
  void error(const char* msg);

  token make_token(token_type type);

  std::string _source;
  char* _ptr;

  int _line;
  int _pos;

  token _cur;
  token _lookahead;

  opmetatable _omtbl;
};

#endif