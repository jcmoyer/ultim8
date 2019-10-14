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

#ifndef EMU_ROMIO_HPP
#define EMU_ROMIO_HPP

#include <cstdint>
#include <cstddef>

class chip8vm;

bool load_rom_from_disk(chip8vm& state, const char* filename);
bool load_rom_from_memory(chip8vm& state, const uint8_t* data, std::size_t size);
bool load_file(chip8vm& state, const char* filename);

#endif