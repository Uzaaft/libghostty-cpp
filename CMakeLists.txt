cmake_minimum_required(VERSION 3.19)

project(
  libghostty_cpp
  LANGUAGES C CXX
)

include(CTest)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- libghostty-vt -----------------------------------------------------------
# Virtual terminal emulator library from Ghostty. Requires `zig` on PATH.
# The Ghostty CMakeLists.txt delegates to `zig build -Demit-lib-vt` to produce
# shared and static libraries. See https://ghostty.org/docs/install/build for the
# required Zig version.
FetchContent_Declare(
  ghostty
  GIT_REPOSITORY https://github.com/ghostty-org/ghostty.git
  GIT_TAG cb28160b5a2fd32d2e1cfeefb01d4297dbca8b18
)
FetchContent_MakeAvailable(ghostty)

option(
  LIBGHOSTTY_CPP_LINK_GHOSTTY_STATIC
  "Link libghostty_cpp to the static libghostty-vt library"
  OFF
)

if(LIBGHOSTTY_CPP_LINK_GHOSTTY_STATIC)
  set(libghostty_cpp_ghostty_vt_build_target ghostty-vt-static)
  set(libghostty_cpp_ghostty_vt_install_target ghostty-vt::ghostty-vt-static)
else()
  set(libghostty_cpp_ghostty_vt_build_target ghostty-vt)
  set(libghostty_cpp_ghostty_vt_install_target ghostty-vt::ghostty-vt)
endif()

add_library(
  libghostty_cpp
  src/c_compat/key.c
  src/c_compat/mouse.c
  src/c_compat/render.c
  src/c_compat/vt.c
  src/build_info.cpp
  src/error.cpp
  src/fmt.cpp
  src/focus.cpp
  src/key.cpp
  src/kitty_graphics.cpp
  src/mouse.cpp
  src/osc.cpp
  src/paste.cpp
  src/render.cpp
  src/screen.cpp
  src/sgr.cpp
  src/sys.cpp
  src/vt.cpp
)

target_sources(
  libghostty_cpp
  PRIVATE
    include/libghostty_cpp/bytes.hpp
    include/libghostty_cpp/build_info.hpp
    include/libghostty_cpp/error.hpp
    include/libghostty_cpp/fmt.hpp
    include/libghostty_cpp/focus.hpp
    include/libghostty_cpp/key.hpp
    include/libghostty_cpp/kitty_graphics.hpp
    include/libghostty_cpp/mouse.hpp
    include/libghostty_cpp/osc.hpp
    include/libghostty_cpp/paste.hpp
    include/libghostty_cpp/render.hpp
    include/libghostty_cpp/screen.hpp
    include/libghostty_cpp/sgr.hpp
    include/libghostty_cpp/style.hpp
    include/libghostty_cpp/sys.hpp
    include/libghostty_cpp/terminal.hpp
    include/libghostty_cpp/vt.hpp
)

target_compile_features(libghostty_cpp PUBLIC cxx_std_20)

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
    $<BUILD_INTERFACE:${libghostty_cpp_ghostty_vt_build_target}>
    $<INSTALL_INTERFACE:${libghostty_cpp_ghostty_vt_install_target}>
)

set(libghostty_cpp_install_cmakedir
    "${CMAKE_INSTALL_LIBDIR}/cmake/libghostty_cpp")

option(
  LIBGHOSTTY_CPP_BUILD_EXAMPLES
  "Build libghostty_cpp example programs"
  OFF
)

if(LIBGHOSTTY_CPP_BUILD_EXAMPLES)
  add_executable(
    libghostty_cpp_build_info_example
    example/build_info/main.cpp
  )

  add_executable(
    libghostty_cpp_focus_example
    example/focus/main.cpp
  )

  target_link_libraries(
    libghostty_cpp_build_info_example
    PRIVATE
      libghostty_cpp
  )

  target_link_libraries(
    libghostty_cpp_focus_example
    PRIVATE
      libghostty_cpp
  )
endif()

option(
  LIBGHOSTTY_CPP_BUILD_GHOSTLING_CPP
  "Build the ghostling_cpp Qt Widgets tech demo"
  OFF
)

if(LIBGHOSTTY_CPP_BUILD_GHOSTLING_CPP AND UNIX)
  find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)

  if(Qt6_FOUND)
    add_executable(
      ghostling_cpp
      example/ghostling_cpp/main.cpp
    )

    target_link_libraries(
      ghostling_cpp
      PRIVATE
        libghostty_cpp
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )

    find_library(libghostty_cpp_util_library util)
    if(libghostty_cpp_util_library)
      target_link_libraries(ghostling_cpp PRIVATE ${libghostty_cpp_util_library})
    endif()

    add_custom_command(
      TARGET ghostling_cpp
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_CURRENT_SOURCE_DIR}/example/ghostling_cpp/fonts/JetBrainsMono-Medium.ttf
              $<TARGET_FILE_DIR:ghostling_cpp>/JetBrainsMono-Medium.ttf
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_CURRENT_SOURCE_DIR}/example/ghostling_cpp/fonts/JetBrainsMono-Regular.ttf
              $<TARGET_FILE_DIR:ghostling_cpp>/JetBrainsMono-Regular.ttf
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_CURRENT_SOURCE_DIR}/example/ghostling_cpp/fonts/OFL.txt
              $<TARGET_FILE_DIR:ghostling_cpp>/OFL.txt
    )

    install(
      TARGETS ghostling_cpp
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(
      FILES
        example/ghostling_cpp/fonts/JetBrainsMono-Medium.ttf
        example/ghostling_cpp/fonts/JetBrainsMono-Regular.ttf
        example/ghostling_cpp/fonts/OFL.txt
      DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
  else()
    message(STATUS "Skipping ghostling_cpp: Qt6 Core/Gui/Widgets not found")
  endif()
endif()

if(BUILD_TESTING)
  add_executable(
    libghostty_cpp_key_smoke_test
    tests/key_smoke.cpp
  )

  add_executable(
    libghostty_cpp_mouse_smoke_test
    tests/mouse_smoke.cpp
  )

  add_executable(
    libghostty_cpp_render_smoke_test
    tests/render_smoke.cpp
  )

  add_executable(
    libghostty_cpp_vt_callbacks_test
    tests/vt_callbacks.cpp
  )

  add_executable(
    libghostty_cpp_vt_modes_test
    tests/vt_modes.cpp
  )

  add_executable(
    libghostty_cpp_vt_grid_ref_test
    tests/vt_grid_ref.cpp
  )

  add_executable(
    libghostty_cpp_vt_state_test
    tests/vt_state.cpp
  )

  add_executable(
    libghostty_cpp_module_smoke_test
    tests/module_smoke.cpp
  )

  target_link_libraries(libghostty_cpp_key_smoke_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_mouse_smoke_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_render_smoke_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_vt_callbacks_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_vt_modes_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_vt_grid_ref_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_vt_state_test PRIVATE libghostty_cpp)
  target_link_libraries(libghostty_cpp_module_smoke_test PRIVATE libghostty_cpp)

  add_test(
    NAME libghostty_cpp_key_smoke_test
    COMMAND libghostty_cpp_key_smoke_test
  )

  add_test(
    NAME libghostty_cpp_mouse_smoke_test
    COMMAND libghostty_cpp_mouse_smoke_test
  )

  add_test(
    NAME libghostty_cpp_render_smoke_test
    COMMAND libghostty_cpp_render_smoke_test
  )

  add_test(
    NAME libghostty_cpp_vt_callbacks_test
    COMMAND libghostty_cpp_vt_callbacks_test
  )

  add_test(
    NAME libghostty_cpp_vt_modes_test
    COMMAND libghostty_cpp_vt_modes_test
  )

  add_test(
    NAME libghostty_cpp_vt_grid_ref_test
    COMMAND libghostty_cpp_vt_grid_ref_test
  )

  add_test(
    NAME libghostty_cpp_vt_state_test
    COMMAND libghostty_cpp_vt_state_test
  )

  add_test(
    NAME libghostty_cpp_module_smoke_test
    COMMAND libghostty_cpp_module_smoke_test
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
