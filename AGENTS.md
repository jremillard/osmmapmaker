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
  3. Execute `ctest --output-on-failure --test-dir bin/release` (or `--test-dir bin/debug`) and make sure every test succeeds before creating a pull request.

- Clean builds by removing the `bin` directory.
- Run `clang-format -i` on all modified C/C++ files before committing.
- The documentation packages for all third-party dependencies are already installed.
