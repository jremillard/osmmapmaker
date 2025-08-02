# Instructions for coding agent On Linux

- **Setup build environment**: See the "Building the Project" section in the README.md file for detailed instructions on setting up the build environment for Windows and Linux/macOS systems.
- **Run unit tests**: 
  1. `ctest --test-dir bin/release`.
  2. `ctest --test-dir bin/valgrind`.
  3. Execute `ctest --output-on-failure --test-dir bin/release` (or `--test-dir bin/valgrind`) and make sure every test succeeds.

- **Advanced testing for development**:
  1. Run each test binary in `bin/valgrind` under `valgrind --tool=memcheck` and `valgrind --tool=helgrind` using the `osmmapmaker_tests.memcheck.supp` and `osmmapmaker_tests.helgrind.supp` suppression files, respectively. No tests should detect `RUNNING_ON_VALGRIND` or be skipped when running under valgrind, and all errors must be investigated.
  2. Update the suppression files if recurring leaks from third-party libraries are discovered so that future runs flag only new problems.
  - Current unit test executable: `osmmapmaker_tests`.

- **Clean builds** by removing the `bin` directory.
- **Run `clang-format -i`** on all modified C/C++ files before committing.
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
