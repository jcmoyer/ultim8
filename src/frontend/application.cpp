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

#include "frontend/application.hpp"
#include "asm/compiler.hpp"
#include "frontend/renderer.hpp"
#include "frontend/romio.hpp"
#include "asm/lexer.hpp"
#include "asm/parser.hpp"
#include "frontend/color_toml.hpp"
#include "frontend/config.hpp"
#include <fmt/format.h>
#include <gl/gl3w.h>
#include <map>

#include <filesystem>

bool get_event_window_id(const SDL_Event& ev, Uint32& id) {
  // not a comprehensive list
  switch (ev.type) {
  case SDL_MOUSEWHEEL:
    id = ev.wheel.windowID;
    return true;
  case SDL_MOUSEBUTTONDOWN:
    id = ev.button.windowID;
    return true;
  case SDL_TEXTINPUT:
    id = ev.text.windowID;
    return true;
  case SDL_KEYDOWN:
  case SDL_KEYUP:
    id = ev.key.windowID;
    return true;
  case SDL_WINDOWEVENT:
    id = ev.window.windowID;
    return true;
  case SDL_DROPFILE:
    id = ev.drop.windowID;
    return true;
  default:
    return false;
  }
}

bool is_event_for(const SDL_Event& ev, Uint32 id) {
  if (Uint32 dest_id; get_event_window_id(ev, dest_id)) {
    return dest_id == id;
  } else {
    return false;
  }
}

void align_to_right(SDL_Window* to, SDL_Window* alignee) {
  int window_x;
  int window_y;
  int window_width;
  int left_border;
  int right_border;
  SDL_GetWindowSize(to, &window_width, nullptr);
  SDL_GetWindowPosition(to, &window_x, &window_y);
  SDL_GetWindowBordersSize(to, nullptr, &left_border, nullptr, &right_border);
  SDL_SetWindowPosition(alignee, window_x + window_width + left_border + right_border, window_y);
}

inline bool map_sdl_key(const std::map<SDL_Keycode, chip8_key>& keymap, SDL_Keycode code, chip8_key& keyvalue) {
  if (auto it = keymap.find(code); it != keymap.end()) {
    keyvalue = it->second;
    return true;
  } else {
    return false;
  }
}

std::vector<uint8_t> compile_demo() {
  static const char* source = R"(
    hires
    ld   v0, 32
    ld   v1, 24
    ld   v2, 32
    ld   v3, 4
    ld   i, texta
    loop:
      disp v0, v1, 0
      add  v0, 16
      add  i, v2
      add  v3, 0xff
      skeq v3, 0
      jmp loop
    halt:
      jmp halt

    texta: data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x90,0x00,0x97,0xB9,0x94,0x05,0x94,0x3D,0x94,0x25,0xE4,0x3D,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00
    textb: data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x7B,0x20,0x42,0x20,0x42,0x20,0x42,0xE0,0x43,0x20,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    textc: data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x04,0xBE,0x07,0xAA,0x04,0xAA,0x04,0xAA,0x04,0xAA,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    textd: data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x39,0xEE,0xA5,0x09,0xBD,0x0F,0xA1,0x08,0xBD,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  )";
  return compile(source);
}

void application::handle_quit(const SDL_Event& ev) {
  running = false;
}

void application::handle_window_event(const SDL_Event& ev) {
  if (ev.window.event == SDL_WINDOWEVENT_CLOSE) {
    running = false;
  } else if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
    update_viewport();
  }
}

void application::handle_key_down(const SDL_Event& ev) {
  if (chip8_key k; map_sdl_key(cfg.input.kmap, ev.key.keysym.sym, k)) {
    chip8->inp.set_key_state(k, true);
  }
  if (ev.key.keysym.sym == SDLK_g) {
    paused = !paused;
    debug->notify_pause_state(paused);
  }
  if (ev.key.keysym.sym == SDLK_h) {
    chip8->step();
  }
  if (ev.key.keysym.sym == cfg.input.reload && filename) {
    load_file(filename->c_str());
  }
  if (ev.key.keysym.sym == cfg.input.toggle_debugger) {
    debug->toggle_visibility();
    // keep focus on this window
    SDL_RaiseWindow(window);
  }
  if (ev.key.keysym.sym == cfg.input.increase_cycles) {
    const int amt = ev.key.keysym.mod & KMOD_LCTRL ? 10000 : 1000;
    cpu_freq.hz(cpu_freq.hz() + amt);
    update_title();
  }
  if (ev.key.keysym.sym == cfg.input.decrease_cycles) {
    const int amt = ev.key.keysym.mod & KMOD_LCTRL ? 10000 : 1000;
    cpu_freq.hz(cpu_freq.hz() - amt);
    if (cpu_freq.hz() < 1000)
      cpu_freq.hz(1000);
    update_title();
  }
  if (ev.key.keysym.sym == SDLK_RETURN && ev.key.keysym.mod & KMOD_LCTRL) {
    toggle_fullscreen();
  }
}

void application::handle_key_up(const SDL_Event& ev) {
  chip8_key input;
  if (map_sdl_key(cfg.input.kmap, ev.key.keysym.sym, input)) {
    chip8->inp.set_key_state(input, false);
  }
}

void application::handle_drop_file(const SDL_Event& ev) {
  load_file(ev.drop.file);
  SDL_free(ev.drop.file);
}

void application::handle_events() {
  SDL_Event ev;
  
  while (running && SDL_PollEvent(&ev)) {
    // dispatch events to debug window and skip them on the main window
    if (is_event_for(ev, debug->window_id())) {
      debug->process_event(ev);
      continue;
    }

    switch (ev.type) {
    case SDL_QUIT:
      handle_quit(ev);
      break;
    case SDL_WINDOWEVENT:
      handle_window_event(ev);
      break;
    case SDL_KEYDOWN:
      handle_key_down(ev);
      break;
    case SDL_KEYUP:
      handle_key_up(ev);
      break;
    case SDL_DROPFILE:
      handle_drop_file(ev);
      break;
    }
  }
}

application::application(int argc, char* argv[]) {
  load_config();

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  window = SDL_CreateWindow("",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    1024,
    512,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  gl = SDL_GL_CreateContext(window);

  gl3wInit();

  render = std::make_unique<renderer>();
  render->set_background_color(cfg.display.render_background);
  render->set_foreground_color(cfg.display.render_foreground);

  window_id = SDL_GetWindowID(window);
  chip8 = std::make_unique<chip8vm>();
  audio = std::make_unique<audio_context>(cfg.audio.frequency, cfg.audio.samples);
  debug = std::make_unique<debugger>();
  debug->set_state(chip8.get());
  debug->on_click_pause = [this]() {
    paused = !paused;
    debug->notify_pause_state(paused);
  };
  debug->on_click_step = [this]() { chip8->step(); };
  if (cfg.debug.visible) {
    debug->show();
  }

  align_to_right(window, debug->window());

  handle_command_line(argc, argv);
}

application::~application() {
  if (window)
    SDL_DestroyWindow(window);
  if (gl)
    SDL_GL_DeleteContext(gl);
}

void application::handle_command_line(int argc, char* argv[]) {
  if (argc > 1) {
    load_file(argv[1]);
  } else {
    auto demo_bytes = compile_demo();
    load_rom_from_memory(*chip8, demo_bytes.data(), demo_bytes.size());
    update_title();
  }
}

void application::run() {
  time_point last = clock::now();

  duration timer_acc = duration{0};
  duration cpu_acc = duration{0};

  const duration profile_delay = duration{1000};
  duration profile_acc = duration{0};
  int cycles_last_second = 0;
  int cycles_per_second = 0;

  // only simulate up to 250ms/iteration
  // it is possible to get massive timeskips e.g. if the user is resizing the window
  const duration MAX_SIM_TIME = duration{250};

  while (running) {
    handle_events();

    time_point now = clock::now();
    if (!paused) {
      duration elapsed = now - last;
      if (elapsed > MAX_SIM_TIME) {
        elapsed = MAX_SIM_TIME;
      }
      timer_acc += elapsed;
      cpu_acc += elapsed;
      profile_acc += elapsed;
    }
    last = now;

    while (timer_acc > timer_freq.dur()) {
      audio->play_tone(chip8->st);
      chip8->dec_timers();
      timer_acc -= timer_freq.dur();
    }

    while (cpu_acc > cpu_freq.dur()) {
      chip8->step();
      ++cycles_last_second;
      cpu_acc -= cpu_freq.dur();
    }

    if (profile_acc > profile_delay) {
      cycles_per_second = cycles_last_second;
      profile_acc -= profile_delay;
      cycles_last_second = 0;
    }

    if (debug->is_visible())
      debug->render(cycles_per_second);

    render_frame();
  }
}

void application::render_frame() {
  SDL_GL_MakeCurrent(window, gl);
  render->render(chip8.get());
  SDL_GL_SwapWindow(window);
}

bool application::load_file(const char* filename_) {
  auto new_state = std::make_unique<chip8vm>();
  bool success = false;
  std::string errmsg;

  try {
    success = ::load_file(*new_state, filename_);
  } catch (const syntax_error& e) {
    if (e.has_help()) {
      errmsg =
        fmt::format("syntax error at {}:{} near `{}': {}\n\n{}", e.line, e.pos, e.context, e.what(), e.help);
    } else {
      errmsg =
        fmt::format("syntax error at {}:{} near `{}': {}", e.line, e.pos, e.context, e.what());
    }
  } catch (const std::exception& e) {
    errmsg = e.what();
  }

  if (success) {
    filename = filename_;
    chip8 = std::move(new_state);
    debug->set_state(chip8.get());
    update_title();
  } else {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unable to load file", errmsg.c_str(), NULL);
  }

  return success;
}

void application::update_title() {
  std::string title;
  if (filename) {
    title = fmt::format("{} - {} cycles/sec", *filename, cpu_freq.hz());
  } else {
    title = fmt::format("[no rom loaded] - {} cycles/sec", cpu_freq.hz());
  }
  SDL_SetWindowTitle(window, title.c_str());
}

void application::load_config() {
  using std::filesystem::path;

  char* base_path = SDL_GetBasePath();
  std::string path_str = base_path;
  SDL_free(base_path);

  auto p = path(path_str) / "config.toml";

  cfg = ::load_config(p.string());
}

void application::update_viewport() {
  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  SDL_GL_MakeCurrent(window, gl);
  glViewport(0, 0, width, height);
}

void application::toggle_fullscreen() {
  if (fullscreen) {
    SDL_SetWindowFullscreen(window, 0);
  } else {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  }
  update_viewport();
  fullscreen = !fullscreen;
}
