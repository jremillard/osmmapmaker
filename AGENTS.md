# Instructions for coding agent

- **Build from scratch** before running tests:
  1. Delete any existing `build` directory (if present).
  2. Configure using `cmake -S . -B build`.
  3. Build with `cmake --build build -j$(nproc)`.

- **Run unit tests** after building:
  1. Change into the build directory using `cd build`.
  2. Execute `ctest --output-on-failure` and make sure every test succeeds before creating a pull request.

- The documentation packages for all third-party dependencies are already installed.
