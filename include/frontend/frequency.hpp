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

#ifndef FRONTEND_FREQUENCY_HPP
#define FRONTEND_FREQUENCY_HPP

#include <chrono>

class frequency {
public:
  using duration = std::chrono::duration<double, std::milli>;

  constexpr explicit frequency(int hz) : _d{0}, _hz{hz} { update_duration(); }

  constexpr int hz() const { return _hz; }
  constexpr void hz(int val) {
    _hz = val;
    update_duration();
  }

  constexpr duration dur() const { return _d; }

private:
  constexpr void update_duration() { _d = duration{1000 / (double)_hz}; }

  duration _d;
  int _hz;
};

#endif
