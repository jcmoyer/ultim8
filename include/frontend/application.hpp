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

#ifndef FRONTEND_APPLICATION_HPP
#define FRONTEND_APPLICATION_HPP

#include "emu/vm.hpp"
#include "frontend/audio.hpp"
#include "frontend/debugger.hpp"
#include "frontend/frequency.hpp"
#include "frontend/config.hpp"
#include <SDL.h>
#include <memory>
#include <optional>
#include <map>

class renderer;

class application {
public:
  application(int argc, char* argv[]);
  ~application();

  void run();

private:
  void handle_events();
  void handle_drop_file(const SDL_DropEvent& ev);
  void handle_key_up(const SDL_KeyboardEvent& ev);
  void handle_key_down(const SDL_KeyboardEvent& ev);
  void handle_window_event(const SDL_WindowEvent& ev);
  void handle_quit(const SDL_QuitEvent& ev);

  void handle_command_line(int argc, char* argv[]);

  void render_frame();

  bool load_file(const char* filename);
  void update_title();

  void load_config();

  void update_viewport();
  void toggle_fullscreen();

private:
  using clock = std::chrono::high_resolution_clock;
  using duration = std::chrono::duration<double, std::milli>;
  using time_point = typename clock::time_point;

  std::unique_ptr<chip8vm> chip8;
  std::unique_ptr<audio_context> audio;
  std::unique_ptr<debugger> debug;
  SDL_Window* window = nullptr;
  SDL_GLContext gl = nullptr;

  std::unique_ptr<renderer> render;

  bool fullscreen = false;

  Uint32 window_id = 0;

  bool running = true;

  // if true, do not advance cpu and timers
  bool paused = false;

  // empty if no file loaded yet
  std::optional<std::string> filename;

  // timing
  frequency cpu_freq{500000};
  frequency timer_freq{60};

  application_config cfg;
};

#endif
