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
  - Current unit test executables include:
    `hello_test`, `textfieldprocessor_test`, `tileoutput_test`,
    `project_schema_test`, `labelpriority_test`, `stylelayer_test`,
    `style_selector_test`, `sublayer_test`, `point_test`, `line_test`,
    `area_test`, `label_test`, `osmdata_test`, `linebreaking_test`,
    `project_load_save_test`.

- Clean builds by removing the `bin` directory.
- Run `clang-format -i` on all modified C/C++ files before committing.
- **Update coverage report** whenever source or test files are modified:
  1. Build the coverage configuration with `cmake --build bin/coverage -j$(nproc)`.
  2. Execute `ctest --output-on-failure --test-dir bin/coverage` and ensure all tests pass.
  3. Collect coverage data using:
     ```
     lcov --capture --directory bin/coverage --output-file bin/coverage/coverage.info
     lcov --remove bin/coverage/coverage.info '/usr/*' '*/tests/*' --output-file bin/coverage/coverage.info
     ```
  4. Generate a sorted line coverage summary with
     ```
     lcov --list bin/coverage/coverage.info | sort -k2 > tests/coverage_report.txt
     ```
     The report must list each source file, covered lines and the percentage covered.
  5. Commit the updated `tests/coverage_report.txt` along with code changes.
- The documentation packages for all third-party dependencies are already installed.
