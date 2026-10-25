cmake_minimum_required(VERSION 3.15)
project(ReactionTrainer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  add_executable(ReactionTrainer WIN32 src/main_win32.cpp)
  target_link_libraries(ReactionTrainer PRIVATE user32 gdi32 ole32 oleaut32 uuid)
  if(MSVC)
    # static runtime: the exe runs on any PC without the Visual C++ redistributable
    set_property(TARGET ReactionTrainer PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_options(ReactionTrainer PRIVATE /O2 /W3)
  else()
    target_link_options(ReactionTrainer PRIVATE -static -static-libgcc -static-libstdc++ -mwindows)
    target_compile_options(ReactionTrainer PRIVATE -O2 -Wall)
  endif()
endif()

# Platform independent logic tests (any OS):  cmake -DRT_BUILD_TESTS=ON ..
option(RT_BUILD_TESTS "Build the headless core tests" OFF)
if(RT_BUILD_TESTS)
  add_executable(core_tests tests/test_core.cpp)
  enable_testing()
  add_test(NAME core_tests COMMAND core_tests)
endif()
