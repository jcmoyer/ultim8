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

#ifndef ASM_ERROR_HPP
#define ASM_ERROR_HPP

#include <stdexcept>
#include <string>
#include <utility>

struct syntax_error : std::runtime_error {
  syntax_error(const char* msg, int line, int pos, std::string context)
      : std::runtime_error(msg), line{line}, pos{pos}, context{std::move(context)} {}

  syntax_error(const char* msg, int line, int pos, std::string context, std::string help)
      : std::runtime_error(msg), line{line}, pos{pos}, context{std::move(context)}, help{std::move(
                                                                                      help)} {}

  bool has_help() const { return help.size(); }

  int line, pos;
  std::string context;
  std::string help;
};

#endif
