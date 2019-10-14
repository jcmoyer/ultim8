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

#include <fstream>
#include <string>
#include <fmt/format.h>

#include "asm/lexer.hpp"
#include "asm/parser.hpp"

std::string read_file(const char* filename) {
  std::ifstream file(filename, std::ios::binary);

  file.seekg(0, std::ios::end);
  std::streampos length = file.tellg();
  file.seekg(0);

  std::string text;
  text.resize(length);

  file.read(text.data(), length);
  return text;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fmt::print("usage: {} <input file> <output file>", argv[0]);
    return EXIT_FAILURE;
  }

  const char* input_filename = argv[1];
  const char* output_filename = argv[2];

  std::string source = read_file(input_filename);
  std::ofstream output(output_filename, std::ios::binary);

  lexer lex(std::move(source));
  parser p(lex);

  try {
    for (const auto& instr : p.parse_instructions()) {
      write_instruction(output, instr);
    }
  } catch (const syntax_error& e) {
    if (e.has_help()) {
      fmt::print("syntax error at {}:{} near `{}': {}\n\n{}", e.line, e.pos, e.context, e.what(), e.help);
    } else {
      fmt::print("syntax error at {}:{} near `{}': {}", e.line, e.pos, e.context, e.what());
    }
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}