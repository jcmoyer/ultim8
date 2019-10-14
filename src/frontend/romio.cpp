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

#include "frontend/romio.hpp"
#include "emu/vm.hpp"
#include "asm/compiler.hpp"
#include <fstream>
#include <cstring>

const char* getext(const char* filename) {
  size_t len = strlen(filename);
  const char* ext = filename + len;
  while (ext > filename && *ext != '.') {
    --ext;
  }
  if (*ext != '.') {
    return nullptr;
  } else {
    return ext;
  }
}

bool load_rom_from_disk(chip8vm& state, const char* filename) {
  std::ifstream file(filename, std::ios::binary);

  if (!file) {
    return false;
  }

  file.seekg(0, std::ios::end);
  const auto program_size = file.tellg();
  file.seekg(0);

  if (static_cast<size_t>(program_size) > chip8vm::PROGRAM_MAX_SIZE) {
    return false;
  }

  // read directly into memory
  uint8_t* program_ptr = &state.memory[chip8vm::PROGRAM_START];
  file.read(reinterpret_cast<char*>(program_ptr), chip8vm::PROGRAM_MAX_SIZE);

  return true;
}

bool load_rom_from_memory(chip8vm& state, const uint8_t* data, std::size_t size) {
  if (size > chip8vm::PROGRAM_MAX_SIZE) {
    return false;
  }

  uint8_t* program_ptr = &state.memory[chip8vm::PROGRAM_START];
  memcpy(program_ptr, data, size);

  return true;
}

bool load_file(chip8vm& state, const char* filename) {
  const char* ext = getext(filename);

  if (!ext) {
    return false;
  }

  if (strcmp(ext, ".ch8") == 0) {
    return load_rom_from_disk(state, filename);
  } else if (strcmp(ext, ".c8s") == 0) {
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
      return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize program_size = file.tellg();
    file.seekg(0);

    std::string program_src;
    program_src.resize(program_size, 0);

    file.read(program_src.data(), program_size);

    auto program = compile(program_src.c_str());

    return load_rom_from_memory(state, program.data(), program.size());
  } else {
    return false;
  }
}