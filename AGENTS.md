# Instructions for coding agent

- **Configure CMake build directories** before running tests:
  1. Delete the `bin` directory if it exists.
  2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release` – optimized binaries.
  3. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug` – includes debug symbols.
  4. Configure a coverage build using `cmake -S . -B bin/coverage -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-O0 -g --coverage" -DCMAKE_CXX_FLAGS="-O0 -g --coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage"` – enables GCC coverage profiling without optimization.
  5. Configure a valgrind build using `cmake -S . -B bin/valgrind -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-O0 -g" -DCMAKE_CXX_FLAGS="-O0 -g"` – debug flags help valgrind report accurate stack traces; optimization isn't required but is usually disabled.

- **Build each directory** with `cmake --build bin/<type> -j$(nproc)`.

- **Run unit tests** after the release and debug builds:
  1. `ctest --test-dir bin/release`.
  2. `ctest --test-dir bin/debug`.
  3. Execute `ctest --output-on-failure --test-dir bin/release` (or `--test-dir bin/debug`) and make sure every test succeeds before creating a pull request.

- Clean builds by removing the `bin` directory.
- Run `clang-format -i` on all modified C/C++ files before committing.
- The documentation packages for all third-party dependencies are already installed.
