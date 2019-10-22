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

#ifndef FRONTEND_DEBUGGER_HPP
#define FRONTEND_DEBUGGER_HPP

#include <SDL.h>
#include "asm/opmeta.hpp"
#include "emu/vm.hpp"
#include <functional>

class application;

struct debugger {
public:
  debugger();
  ~debugger();

  void process_event(const SDL_Event& ev);
  void set_state(chip8vm* ptr);

  void render(int cps);

  bool is_visible() const;
  void show();
  void hide();
  void toggle_visibility();

  void set_position(int x, int y);

  void notify_pause_state(bool paused) { _paused = paused; }

  SDL_Window* window() const { return _window; }
  Uint32 window_id() const { return _windowid; }

public:
  std::function<void()> on_click_pause;
  std::function<void()> on_click_step;

private:
  SDL_Window* _window;
  SDL_GLContext _gl;
  chip8vm* _chip8;
  opmetatable _omtbl;
  Uint32 _windowid;
  bool _paused = false;
};

#endif