Please find the code on [GitHub](https://github.com/Muirey03/advanced-project):

	git clone --recursive https://github.com/Muirey03/advanced-project.git

## Directory Structure

- `llvm/`
 	- `llvm/clang/lib/StaticAnalyzer/Checkers/RacyUAFChecker.cpp`: The implementation of the checker
- `tests/`
	- `(data|general)_race[0-9]+/`: tests for manufactured solutions
	- `include/test_utils.h`: header including annotations and shared code for the tests
	- `chromeos-powervr/`: test for Chromium Issue 400664
	- `flow_divert/`: test for CVE-2022-26757
	- `xnu`: test for XNU

## Running Tests

Please use with CMake, following the LLVM build instructions: https://llvm.org/docs/GettingStarted.html

**To run all tests excluding XNU:** `cmake --build cmake-build-release/ --target all_tests`.

**To test XNU:**
- Build XNU according to the instructions at: https://github.com/blacktop/darwin-xnu-build
- Create a Makefile for the test with `./create_makefile.sh`
- Run the tests with `./run_checker.sh`
