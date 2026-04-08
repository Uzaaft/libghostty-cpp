# libghostty-cpp

May the Lord have mercy upon my soul.

Minimal C++ wrapper around `libghostty-vt`, with the high-level API shape aligned closely to `libghostty-rs` while leaning into RAII and value-typed C++ APIs.

Current wrapper coverage:

- `Terminal` basics, byte-oriented VT IO, typed resize/value accessors, typed viewport scrolling, active screen and scrollback metadata queries, cursor and mode queries, safe title and pwd accessors with explicit borrowed views, point-based grid refs for hit-testing/traversal, typed color palette accessors, and effect callbacks
- `RenderState` basics for render metadata, colors, dirty tracking, and row/cell traversal
- `key` encoder and key event wrappers for VT input encoding
- `mouse` encoder and mouse event wrappers for VT mouse encoding
- `focus` helpers for focus gained/lost event encoding
- `paste` helpers for safe paste checks and bracketed paste encoding
- `build_info` queries for the bundled Ghostty build configuration and version
- `osc` streaming parser for operating system command sequences
- `sgr` parser for typed SGR attribute decoding
- `fmt` formatter for plain text, VT, and HTML exports from terminal state

The public entry points are the headers under `include/libghostty_cpp/`. Most terminal-facing APIs are available through `terminal.hpp` or `vt.hpp`.

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
ctest --test-dir build --output-on-failure
```

The Ghostty dependency is pinned to commit `0a492fdb331f1e0be29aedbcc78c3c852cb437f2`, which matches the currently targeted `libghostty-rs` API generation.

## Optional Example

`ghostling_cpp` is a Qt Widgets tech demo that exercises the higher-level wrappers, including key, mouse, focus, title, render, and bracketed paste handling.

If Qt6 Core, Gui, and Widgets are available, it is built automatically as part of the default build. You can also build it explicitly with:

```sh
cmake --build build --target ghostling_cpp
```

## API Sketch

```cpp
#include "libghostty_cpp/focus.hpp"
#include "libghostty_cpp/fmt.hpp"
#include "libghostty_cpp/paste.hpp"
#include "libghostty_cpp/terminal.hpp"

libghostty_cpp::Terminal terminal(libghostty_cpp::GridSize{80, 24}, 1000);
terminal.resize(libghostty_cpp::GridSize{80, 24}, libghostty_cpp::CellSize{9, 18});
terminal.vt_write(libghostty_cpp::ByteView{"hello from ghostty"});

const auto focus = libghostty_cpp::focus::encode(libghostty_cpp::focus::Event::Gained);
const auto paste = libghostty_cpp::paste::encode("printf 'hi'\n", true);
const std::string title = terminal.title();

libghostty_cpp::fmt::Formatter formatter(
  terminal,
  libghostty_cpp::fmt::FormatterOptions{libghostty_cpp::fmt::Format::Plain, false, false}
);

const std::string snapshot = formatter.format_string();
```
