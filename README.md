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

## Quick Start

```cpp
#include "libghostty_cpp/terminal.hpp"
#include "libghostty_cpp/render.hpp"

#include <cstdio>

int main() {
    // Create a terminal with 80 columns, 24 rows, and scrollback.
    libghostty_cpp::Terminal terminal({80, 24, 10'000});

    // Register an effect handler for PTY write-back (e.g. query responses).
    terminal.on_pty_write([](const libghostty_cpp::Terminal&, libghostty_cpp::ByteView data) {
        std::printf("PTY response: %zu bytes\n", data.size);
    });

    // Feed VT-encoded data into the terminal.
    terminal.vt_write(libghostty_cpp::ByteView{"Hello, \x1b[1;32mworld\x1b[0m!\r\n"});
    terminal.vt_write(libghostty_cpp::ByteView{"\x1b[38;2;255;128;0morange text\x1b[0m\r\n"});

    // Capture a render snapshot and iterate rows/cells.
    libghostty_cpp::RenderState render_state;
    libghostty_cpp::RowIterator rows;
    libghostty_cpp::CellIterator cells;

    render_state.update(terminal);
    rows.bind(render_state);

    while (rows.next()) {
        cells.bind(rows);
        while (cells.next()) {
            const auto graphemes = cells.graphemes();
            // … render each cell
        }
    }
}
```
