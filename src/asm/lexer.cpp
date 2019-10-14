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

#include "asm/lexer.hpp"
#include <charconv>

bool is_alpha(char ch) {
  return ch >= 'a' && ch <= 'z';
}

bool is_num(char ch) {
  return ch >= '0' && ch <= '9';
}

bool is_binnum(char ch) {
  return ch == '0' || ch == '1';
}

bool is_hexnum(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

bool is_alphanum(char ch) {
  return is_alpha(ch) || is_num(ch);
}

bool is_name_char(char ch) {
  return is_alpha(ch) || is_num(ch) || ch == '_';
}

bool is_whitespace(char ch) {
  return ch == '\t' || ch == ' ';
}

bool is_variable_name(std::string_view s) {
  return s.size() == 2 && (s[0] == 'v' || s[0] == 'V') &&
         ((s[1] >= '0' && s[1] <= '9') || (s[1] >= 'a' && s[1] <= 'f') ||
           (s[1] >= 'A' && s[1] <= 'F'));
}

int variable_id(std::string_view s) {
  switch (s[1]) {
  case '0': return 0;
  case '1': return 1;
  case '2': return 2;
  case '3': return 3;
  case '4': return 4;
  case '5': return 5;
  case '6': return 6;
  case '7': return 7;
  case '8': return 8;
  case '9': return 9;
  case 'a': case 'A': return 0xa;
  case 'b': case 'B': return 0xb;
  case 'c': case 'C': return 0xc;
  case 'd': case 'D': return 0xd;
  case 'e': case 'E': return 0xe;
  case 'f': case 'F': return 0xf;
  default:
    return -1;
  }
}

template <typename Integer>
std::from_chars_result from_chars_sv(std::string_view view, Integer& i, int base = 10) {
  return std::from_chars(
    view.data(), view.data() + view.size(), i, base
  );
}

lexer::lexer(std::string source)
    : _source{std::move(source)}, _ptr{_source.data()}, _line{1}, _pos{1}, _lookahead{
                                                                             token_type::uninit} {
}

void lexer::next() {
  if (_lookahead.type != token_type::uninit) {
    _cur = _lookahead;
    _lookahead.type = token_type::uninit;
  } else {
    _cur = lex_one();
  }
}

char lexer::read_one() {
  ++_pos;
  ++_ptr;
  return *_ptr;
}

token lexer::lex_one() {
  for (;;) {
    char ch = *_ptr;
    switch (ch) {
    case ';':
      read_comment();
      break;
    case '\r':
      read_one();
      break;
    case '\n':
      read_one();
      new_line();
      break;
    case ' ':
    case '\t':
      read_one();
      break;
    case ':':
      read_one();
      return make_token(token_type::colon);
    case ',':
      read_one();
      return make_token(token_type::comma);
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return read_number();
    case '\0':
      return make_token(token_type::eos);
    default:
      if (is_alpha(ch) || ch == '_') {
        return read_name();
      } else {
        error("invalid character");
      }
    }
  }
}

void lexer::new_line() {
  ++_line;
  _pos = 1;
}

void lexer::read_comment() {
  while (*_ptr != '\r' && *_ptr != '\n') {
    ++_ptr;
  }
}

token lexer::read_number() {
  enum num_type { dec, hex, bin };

  num_type num_ty = dec;
  char* start = _ptr;

  // determine what kind of number we're dealing with
  if (*_ptr == '0' && *(_ptr + 1) == 'x') {
    num_ty = hex;
    _ptr += 2;
    start = _ptr;
  } else if (*_ptr == '0' && *(_ptr + 1) == 'b') {
    num_ty = bin;
    _ptr += 2;
    start = _ptr;
  }

  if (num_ty == hex) {
    while (is_hexnum(read_one())) {
    }
  } else if (num_ty == bin) {
    while (is_binnum(read_one())) {
    }
  } else {
    while (is_num(read_one())) {
    }
  }

  char* end = _ptr;

  std::string_view view{start, static_cast<std::size_t>(end - start)};

  int value = 0;

  switch (num_ty) {
  case dec: from_chars_sv(view, value, 10); break;
  case hex: from_chars_sv(view, value, 16); break;
  case bin: from_chars_sv(view, value, 2); break;
  }

  token t = make_token(token_type::number);
  t.span = view;
  t.numeric_value = value;
  return t;
}

token lexer::read_name() {
  token t = make_token(token_type::text);
  char* start = _ptr;
  while (is_name_char(read_one())) {}
  char* end = _ptr;
  t.span = std::string_view{start, static_cast<std::size_t>(end - start)};
  if (t.span == "i") {
    t.type = token_type::i;
  } else if (t.span == "dt") {
    t.type = token_type::dt;
  } else if (t.span == "st") {
    t.type = token_type::st;
  } else if (is_variable_name(t.span)) {
    t.type = token_type::variable;
    t.numeric_value = variable_id(t.span);
  } else if (_omtbl.is_mnemonic(t.span)) {
    t.type = token_type::mnemonic;
  }
  return t;
}

void lexer::error(const char* msg) {
  throw syntax_error(msg, _line, _pos, std::string(current().span));
}

token lexer::make_token(token_type type) {
  token t;
  t.type = type;
  t.location.line = _line;
  t.location.pos = _pos;
  return t;
}