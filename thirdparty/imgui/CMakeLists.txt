add_library(imgui imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_impl_sdl.cpp imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(imgui SDL2-static gl3w)