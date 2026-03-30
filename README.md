# libghostty-cpp

May the Lord have mercy upon my soul.

Minimal C++ wrapper around `libghostty-vt`.

Current wrapper coverage:

- `Terminal` basics, resize, cursor queries, and effect callbacks
- `RenderState` basics for render metadata, colors, dirty tracking, and row/cell traversal
- `key` encoder and key event wrappers for VT input encoding
- `mouse` encoder and mouse event wrappers for VT mouse encoding

## Building

Requirements:

- [CMake](https://cmake.org/) 3.19+
- [Ninja](https://ninja-build.org/)
- A C++ compiler with C++17 support
- [Zig](https://ziglang.org/) 0.15.x on `PATH` (`flake.nix` pins `0.15.2`)
- `git` for CMake `FetchContent`
- macOS: Command Line Tools or Xcode


```
cmake -B build -G Ninja
cmake --build build
```
