# libghostty-cpp

May the Lord have mercy upon my soul.

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
