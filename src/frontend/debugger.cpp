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

#include "frontend/debugger.hpp"
#include "emu/instruction.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include <gl/gl3w.h>
#include <fmt/format.h>

debugger::debugger() {
  _window = SDL_CreateWindow("debugger",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    300,
    512,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
  _gl = SDL_GL_CreateContext(_window);
  _windowid = SDL_GetWindowID(_window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForOpenGL(_window, _gl);
  ImGui_ImplOpenGL3_Init("#version 130");
}

debugger::~debugger() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  SDL_GL_DeleteContext(_gl);
  SDL_DestroyWindow(_window);
}

bool debugger::is_visible() const {
  Uint32 flags = SDL_GetWindowFlags(_window);
  return flags & SDL_WINDOW_SHOWN;
}

void debugger::show() {
  SDL_ShowWindow(_window);
}

void debugger::hide() {
  SDL_HideWindow(_window);
}

void debugger::toggle_visibility() {
  if (is_visible()) {
    hide();
  } else {
    show();
  }
}

void debugger::set_state(chip8vm* chip8) {
  _chip8 = chip8;
}

void debugger::render(int cps) {
  SDL_GL_MakeCurrent(_window, _gl);

  ImGuiIO& io = ImGui::GetIO();

  ImGui::StyleColorsDark();

  int window_w, window_h;
  SDL_GetWindowSize(_window, &window_w, &window_h);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(_window);
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_w), static_cast<float>(window_h)));
  ImGui::GetStyle().WindowRounding = 0.0f;

  ImGui::Begin(
    "debug", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  ImGui::Text("cycles/sec: %d", cps);
  ImGui::Text("cpu status: %s", cpu_status_str(_chip8->status));

  ImGui::BeginGroup();
  ImGui::Columns(2, NULL, false);
  for (int y = 0; y < 8; y++) {
    ImGui::Text("v%x = %02x", y, _chip8->variables[y]);
  }
  ImGui::NextColumn();
  for (int y = 8; y < 16; y++) {
    ImGui::Text("v%x = %02x", y, _chip8->variables[y]);
  }
  ImGui::Columns(1);
  ImGui::Spacing();
  ImGui::Text("dt = %02x", _chip8->dt);
  ImGui::Text("st = %02x", _chip8->st);
  ImGui::Text(" i = %04x", _chip8->i);
  ImGui::EndGroup();

  ImGui::Separator();

  ImGui::BeginGroup();
  for (int pco = _chip8->pc - 16, y = 0; pco <= _chip8->pc + 16; pco += 2, ++y) {
    uint16_t* instr = reinterpret_cast<uint16_t*>(&_chip8->memory[pco]);
    const auto dinstr = decode(*instr);
    const auto* meta = _omtbl.find_opcode(dinstr.opcode);
    std::string label;
    if (meta) {
      switch (meta->parameter_count()) {
      case 0:
        label = fmt::format("{:6}", meta->mnemonic);
        break;
      case 1:
        label = fmt::format("{:6} {}", meta->mnemonic, param_string(*meta, dinstr.opcode, 0));
        break;
      case 2:
        label = fmt::format("{:6} {}, {}",
          meta->mnemonic,
          param_string(*meta, dinstr.opcode, 0),
          param_string(*meta, dinstr.opcode, 1));
        break;
      case 3:
        label = fmt::format("{:6} {}, {}, {}",
          meta->mnemonic,
          param_string(*meta, dinstr.opcode, 0),
          param_string(*meta, dinstr.opcode, 1), 
          param_string(*meta, dinstr.opcode, 2));
        break;
      default:
        assert(false);
        break;
      }
    } else {
      label = "[unknown]";
    }
    ImVec4 color;
    if (pco == _chip8->pc) {
      color = ImVec4(1, 1, 0, 1);
    } else {
      color = ImVec4(1, 1, 1, 1);
    }
    ImGui::TextColored(color, "%04x", pco);
    ImGui::SameLine();
    ImGui::TextColored(color, "%04x", decode(*instr).opcode);
    ImGui::SameLine();
    ImGui::TextColored(color, label.c_str());
  }
  ImGui::EndGroup();

  ImGui::Separator();

  if (ImGui::Button("Step")) {
    on_click_step();
  }

  ImGui::SameLine();

  const char* pause_resume = _paused ? "Resume" : "Pause";

  if (ImGui::Button(pause_resume)) {
    on_click_pause();
  }

  ImGui::End();

  ImGui::Render();
  glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  SDL_GL_SwapWindow(_window);
}

void debugger::set_position(int x, int y) {
  SDL_SetWindowPosition(_window, x, y);
}

void debugger::process_event(const SDL_Event& ev) {
  if (ImGui_ImplSDL2_ProcessEvent(&ev)) {
    return;
  }

  if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE) {
    hide();
  }
}