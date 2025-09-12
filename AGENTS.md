AGENTS: Build, Test, and Code Style Guide

1) Build
- Xcode: open example_noise/example_noise.xcodeproj and build the "example_noise" scheme (Debug/Release).
- Makefile (openFrameworks): from example_noise/, run:
  - Debug: make
  - Release: make Release
  - Clean: make clean

2) Run
- App runs as a desktop app; no CLI args required.

3) Tests/Lint
- No unit tests or linter configured in repo. If adding tests, prefer Catch2 or GoogleTest and integrate with the Makefile/Xcode scheme. To run a single test, use your framework's filter flag (e.g., Catch2: --test-case "Name").

4) C++ Style (C++17+, openFrameworks)
- Imports: include openFrameworks headers with #include "ofMain.h"; local headers with quotes; system headers with <>.
- Formatting: clang-format (Google or LLVM style). 100-col limit; 4-space indent; braces on same line for functions, classes, and control blocks.
- Types: use explicit types (float, int). Prefer glm::vec* for math. Use const & for read-only params; use auto only when type is obvious.
- Naming: Types PascalCase (ParticleField), methods camelCase (updatePositions), constants UPPER_SNAKE_CASE (PARTICLE_COUNT), file names PascalCase.h/.cpp.
- Error handling: prefer assertions (OF_ASSERT or assert) for invariants; guard GPU calls; log via ofLog (OF_LOG_ERROR/WARNING/NOTICE). Fail fast in setup() and bail safely.
- Performance: avoid heap in tight loops; pre-allocate std::vector; use ofBufferObjects and shaders for heavy work.
- Shaders: keep GLSL in strings/headers here; validate link status; log compile errors.

5) Project Conventions
- Addon headers live in src/; example app under example_noise/.
- No Cursor (.cursor/rules) or Copilot rules present.
- If introducing CI, add: build (make Release), optional unit tests, and a clang-format check.
