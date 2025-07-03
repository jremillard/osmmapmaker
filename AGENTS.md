# Instructions for coding agent

- **Build from scratch** before running tests:
  1. Delete the `bin` directory if it exists.
  2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release`.
  3. Build with `cmake --build bin/release -j$(nproc)`.
  4. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug`.
  5. Build with `cmake --build bin/debug -j$(nproc)`.

- **Run unit tests** after both builds:
  1. `ctest --test-dir bin/release`.
  2. `ctest --test-dir bin/debug`.

- Clean builds by removing the `bin` directory.
- The documentation packages for all third-party dependencies are already installed.
