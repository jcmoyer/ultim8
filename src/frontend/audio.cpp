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

#include "frontend/audio.hpp"

std::vector<float> build_square_wave(float v, int freq, int relative_freq) {
  const int samples_per_square = relative_freq / freq;

  std::vector<float> samples;
  samples.resize(samples_per_square);
  float* data = samples.data();
  for (int i = 0; i < samples_per_square; ++i) {
    if (i < samples_per_square / 2)
      *data = -v;
    else
      *data = +v;
    ++data;
  }
  return samples;
}

audio_context::audio_context(int frequency, uint16_t samples) {
  SDL_AudioSpec want;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = frequency;
  want.format = AUDIO_F32;
  want.channels = 1;
  want.samples = samples;
  want.callback = audio_callback;
  want.userdata = this;
  device = SDL_OpenAudioDevice(NULL, 0, &want, &spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

  // 440hz = A4
  buffer = build_square_wave(0.2f, 440, spec.freq);
  buffer_index = 0;

  SDL_PauseAudioDevice(device, 0);
}

audio_context::~audio_context() {
  SDL_CloseAudioDevice(device);
}

void audio_context::play_tone(int timer) {
  st = timer;
}

void audio_context::audio_callback(void* userdata, Uint8* stream, int len) {
  audio_context* self = static_cast<audio_context*>(userdata);
  float* data = reinterpret_cast<float*>(stream);

  bool zero = self->st == 0;

  for (int i = 0; i < static_cast<int>(len / sizeof(float)); ++i) {
    if (zero) {
      *data++ = 0;
    } else {
      *data++ = self->buffer[self->buffer_index++];
      self->buffer_index %= self->buffer.size();
    }
  }
}