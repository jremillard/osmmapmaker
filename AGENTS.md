# Instructions for coding agent

- **Configure CMake build directories** before running tests:
  1. Delete the `bin` directory if it exists.
  2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release` – optimized binaries.
  3. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug` – includes debug symbols.
  4. Configure a coverage build using `cmake -S . -B bin/coverage -DOSMMAPMAKER_ENABLE_COVERAGE=ON` – automatically sets a Debug build and enables GCC coverage profiling.
  5. Configure a valgrind build using `cmake -S . -B bin/valgrind -DOSMMAPMAKER_ENABLE_VALGRIND=ON` – automatically sets a Debug build with flags suitable for valgrind analysis.

- **Build each directory** with `cmake --build bin/<type> -j$(nproc)`.

- **Run unit tests** after the release and debug builds:
  1. `ctest --test-dir bin/release`.
  2. `ctest --test-dir bin/debug`.
  3. Execute `ctest --output-on-failure --test-dir bin/release` (or `--test-dir bin/debug`) and make sure every test succeeds before creating a pull request.

- Clean builds by removing the `bin` directory.
- Run `clang-format -i` on all modified C/C++ files before committing.
- The documentation packages for all third-party dependencies are already installed.
