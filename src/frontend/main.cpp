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

#include <SDL.h>
#include <SDL_main.h>
#include "frontend/application.hpp"

bool run(int argc, char* argv[]) {
  try {
    application app(argc, argv);
    app.run();
  } catch (const config_error& e) {
    SDL_Log("error reading config file: %s", e.what());
    return false;
  } catch (const std::exception& e) {
    SDL_Log("%s", e.what());
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("%s", SDL_GetError());
    return EXIT_FAILURE;
  }

  int status = run(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
  
  SDL_Quit();

  return status;
}