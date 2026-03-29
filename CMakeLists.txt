cmake_minimum_required(VERSION 3.19)

project(
  libghostty_cpp
  LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- libghostty-vt -----------------------------------------------------------
# Virtual terminal emulator library from Ghostty. Requires `zig` on PATH.
# The Ghostty CMakeLists.txt delegates to `zig build lib-vt` to produce the
# shared library. See https://ghostty.org/docs/install/build for the
# required Zig version.
FetchContent_Declare(
  ghostty
  GIT_REPOSITORY https://github.com/ghostty-org/ghostty.git
  GIT_TAG bebca84668947bfc92b9a30ed58712e1c34eee1d
)
FetchContent_MakeAvailable(ghostty)

add_library(
  libghostty_cpp
  src/c_compat/vt.c
  src/error.cpp
  src/vt.cpp
)

target_sources(
  libghostty_cpp
  PUBLIC
    include/libghostty_cpp/error.hpp
    include/libghostty_cpp/vt.hpp
)

target_compile_features(libghostty_cpp PUBLIC cxx_std_17)

target_include_directories(
  libghostty_cpp
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${ghostty_SOURCE_DIR}/include
)

target_link_libraries(libghostty_cpp PUBLIC ghostty-vt)

install(
  TARGETS libghostty_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)
