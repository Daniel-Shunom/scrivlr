# scrivlr

C++26 with named modules. Requires CMake 3.30+, Ninja, and a compiler
supporting C++26 and CMake module dependency scanning. Verified with GCC 15.2.

```sh
cmake --preset debug
cmake --build --preset debug
```

Run `build/debug/scrivlr.exe` on Windows or `build/debug/scrivlr` elsewhere.
Use the `release` preset for optimized builds.

`src/scrivlr_lib.cppm` exports `scrivlr_lib`. Consumers link `scrivlr::lib`
and use `import scrivlr_lib;` for `scrivlr::BoundingBox` and `scrivlr::Drawing`.
`src/main.cpp` is the application entry point.

Existing naming conventions remain: PascalCase types, camelCase functions,
snake_case variables and members, UPPER_CASE constants. Formatting and lint
configuration lives in `.editorconfig`, `.clang-format`, and `.clang-tidy`.
clangd reads the debug compilation database; module editor support varies.

`vendor/clay.h` remains unchanged and is available through the optional
`scrivlr::clay` target. The application does not yet invoke Clay.

GLAD 0.1.36 is vendored in `vendor/glad` with the downloaded API selection:
OpenGL 4.0 compatibility, GLES 1.0/3.2, and GLSC 2.0. Its C loader is compiled
once by `scrivlr::glad`, which the application already links.

Use `import glad;` to access GL types, loader functions, function pointers
such as `glClear`, and `constexpr` constants such as `GL_COLOR_BUFFER_BIT`.
Other CMake targets using this module must link `scrivlr::glad` too.
After creating and making an OpenGL context current, initialize GLAD with
`gladLoadGL()` or `gladLoadGLLoader(your_proc_address_callback)` and check for
a nonzero result before calling OpenGL functions. GLAD does not create a window
or context. For GLFW, define `GLFW_INCLUDE_NONE` before including its header.

Do not also include GLAD/OpenGL headers in a translation unit importing `glad`.
Module constants are C++ values, so they cannot be used in preprocessor `#if`.
After replacing the vendored GLAD 1 files, regenerate the facade with
`python tools/generate_glad_module.py`.

Run `ctest --test-dir build/debug --output-on-failure` for the module import,
constant, function alias, and loader failure-path smoke test (no context needed).

GLFW 3.4 is fetched from its official source archive by CMake, verified with
SHA-256, and built as a static library using your compiler. The first configure
requires internet access. No GLFW DLL installation is needed.

The application links the `glfw` target and defines `GLFW_INCLUDE_NONE` to keep
its header compatible with `import glad;`. Use `#include <GLFW/glfw3.h>` for
GLFW calls. `src/main.cpp` creates an OpenGL 4.0 core context, initializes GLAD
with GLFW's procedure resolver, and runs a resize-aware clear/swap/event loop.
Press Escape or close the window to exit.

Run `build/debug/scrivlr.exe --smoke-test` to create a hidden window, initialize
OpenGL, render one frame, and exit. This check requires a working graphics
driver and desktop session.
