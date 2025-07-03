# Instructions for coding agent

- **Configure CMake build directories** before running tests:
  1. Delete the `bin` directory if it exists.
  2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release` – optimized binaries.
  3. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug` – includes debug symbols.
  4. Configure a coverage build using `cmake -S . -B bin/coverage -DOSMMAPMAKER_ENABLE_COVERAGE=ON` – automatically sets a Debug build and enables GCC coverage profiling.
  5. Configure a valgrind build using `cmake -S . -B bin/valgrind -DOSMMAPMAKER_ENABLE_VALGRIND=ON` – automatically sets a Debug build with flags suitable for valgrind analysis.

- **Build each directory** with `cmake --build bin/<type> -j$(nproc)`.

- **Run unit tests** after the release and valgrind builds:
  1. `ctest --test-dir bin/release`.
  2. `ctest --test-dir bin/valgrind`.
  3. Execute `ctest --output-on-failure --test-dir bin/release` (or `--test-dir bin/valgrind`) and make sure every test succeeds.
  4. Run each test binary in `bin/valgrind` under both `valgrind --tool=memcheck` and `valgrind --tool=helgrind` using the `valgrind.supp` suppression file. No tests should detect `RUNNING_ON_VALGRIND` or be skipped when running under valgrind, and all errors must be investigated.
  5. Update `valgrind.supp` if recurring leaks from third-party libraries are discovered so that future runs flag only new problems.

- Clean builds by removing the `bin` directory.
- Run `clang-format -i` on all modified C/C++ files before committing.
- The documentation packages for all third-party dependencies are already installed.
