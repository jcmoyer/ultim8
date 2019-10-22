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

#ifndef FRONTEND_AUDIO_HPP
#define FRONTEND_AUDIO_HPP

#include <vector>
#include <SDL.h>
#include <cstdint>
#include <atomic>

class audio_context {
public:
  audio_context(int frequency, uint16_t samples);
  ~audio_context();

  void play_tone(int id);

private:
  SDL_AudioDeviceID  device;
  SDL_AudioSpec      spec;
  std::atomic<int>   st = 0;
  std::vector<float> buffer;
  std::size_t        buffer_index;

  static void audio_callback(void* userdata, Uint8* stream, int len);
};

#endif
