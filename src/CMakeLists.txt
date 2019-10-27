add_library(
  ultim8asm
  asm/lexer.cpp
  asm/opmeta.cpp
  asm/parser.cpp
  asm/compiler.cpp
)
set_target_properties(ultim8asm PROPERTIES CXX_STANDARD 17)
target_include_directories(ultim8asm PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_library(
  ultim8emu
  emu/vm.cpp
  emu/framebuffer.cpp
)
set_target_properties(ultim8emu PROPERTIES CXX_STANDARD 17)
target_include_directories(ultim8emu PRIVATE "${CMAKE_SOURCE_DIR}/include")

add_executable(
  ultim8
  frontend/main.cpp
  frontend/audio.cpp
  frontend/debugger.cpp
  frontend/application.cpp
  frontend/romio.cpp
  frontend/renderer.cpp
  frontend/config.cpp
  frontend/keymap.cpp
)
set_target_properties(ultim8 PROPERTIES CXX_STANDARD 17)
target_include_directories(ultim8 PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(ultim8 PUBLIC SDL2-static SDL2main ultim8asm ultim8emu gl3w imgui fmt toml11)

add_executable(ultim8c assemble.cpp)
set_target_properties(ultim8c PROPERTIES CXX_STANDARD 17)
target_include_directories(ultim8c PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(ultim8c PRIVATE ultim8asm fmt)

# min/max macros from winapi collide with std::min/max
if(WIN32)
  target_compile_definitions(ultim8 PRIVATE NOMINMAX)
endif()

set(
  ULTIM8_CXX_FLAGS
  $<$<CXX_COMPILER_ID:GNU>:-Wall>
  # MSVC requires this flag to set the correct value for __cplusplus;
  # required for string_view support in toml11
  $<$<CXX_COMPILER_ID:MSVC>:/Zc\:__cplusplus>
)

target_compile_options(ultim8 PRIVATE ${ULTIM8_CXX_FLAGS})
target_compile_options(ultim8emu PRIVATE ${ULTIM8_CXX_FLAGS})
target_compile_options(ultim8asm PRIVATE ${ULTIM8_CXX_FLAGS})
target_compile_options(ultim8c PRIVATE ${ULTIM8_CXX_FLAGS})

install(TARGETS ultim8 ultim8c RUNTIME DESTINATION .)
