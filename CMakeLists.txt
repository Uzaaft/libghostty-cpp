cmake_minimum_required(VERSION 3.19)

project(
  libghostty_cpp
  LANGUAGES C CXX
)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

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
  src/c_compat/render.c
  src/c_compat/vt.c
  src/error.cpp
  src/render.cpp
  src/vt.cpp
)

target_sources(
  libghostty_cpp
  PRIVATE
    include/libghostty_cpp/error.hpp
    include/libghostty_cpp/render.hpp
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

target_link_libraries(
  libghostty_cpp
  PUBLIC
    $<BUILD_INTERFACE:ghostty-vt>
    $<INSTALL_INTERFACE:ghostty-vt::ghostty-vt>
)

set(libghostty_cpp_install_cmakedir
    "${CMAKE_INSTALL_LIBDIR}/cmake/libghostty_cpp")

if(BUILD_TESTING)
  add_executable(
    libghostty_cpp_render_smoke_test
    tests/render_smoke.cpp
  )

  add_executable(
    libghostty_cpp_vt_callbacks_test
    tests/vt_callbacks.cpp
  )

  target_link_libraries(libghostty_cpp_render_smoke_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_vt_callbacks_test PRIVATE libghostty_cpp)

  add_test(
    NAME libghostty_cpp_render_smoke_test
    COMMAND libghostty_cpp_render_smoke_test
  )

  add_test(
    NAME libghostty_cpp_vt_callbacks_test
    COMMAND libghostty_cpp_vt_callbacks_test
  )
endif()

install(
  TARGETS libghostty_cpp
  EXPORT libghostty_cppTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
  cmake/libghostty_cpp-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/libghostty_cpp-config.cmake
  INSTALL_DESTINATION ${libghostty_cpp_install_cmakedir}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libghostty_cpp-config.cmake
  DESTINATION ${libghostty_cpp_install_cmakedir}
)

install(
  EXPORT libghostty_cppTargets
  FILE libghostty_cppTargets.cmake
  NAMESPACE libghostty_cpp::
  DESTINATION ${libghostty_cpp_install_cmakedir}
)
