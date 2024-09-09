#!/bin/bash

mkdir -p "build"

# Define color constants
RED='\x1b[91m'
GREEN='\x1b[92m'
YELLOW='\x1b[93m'
BLUE='\x1b[94m'
MAGENTA='\x1b[95m'
CYAN='\x1b[96m'
WHITE='\x1b[97m'
NC='\x1b[0m'  # No Color

PREFIX="[❯]"

cmake_options="-DLOG_WARN=ON
-DLOG_INFO=ON
-DLOG_TRACE=ON
-DLOG_DEBUG=ON
-DLOG_FILES=ON
-DLOG_TEST=OFF"

test_names=()
handle_test_names=false

# Flags
fnag_run_cmake=false
flag_run_make=false
flag_run_testbed=false

# Function to print colored text
color_print() {
  local color="$1"
  local message="$2"
  echo -e "${PREFIX}  ${color}${message}${NC}"
}

run_cmake() {
  pushd build || { echo "Failed to enter build directory"; exit 1; }
  cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $cmake_options .. || { echo "CMake configuration failed"; popd; exit 1; }
  popd
}

run_make() {
  pushd build || { echo "Failed to enter build directory"; exit 1; }
  make || { echo "Compilation with make failed"; popd; exit 1; }
  popd
}

run_testbed() {
  pushd build || { echo "Failed to enter build directory"; exit 1; }
  pushd testbed || { echo "Failed to enter testbed directory"; popd; exit 1; }
  ./testbed
  popd
  popd
}

run_tests() {
  pushd build || { echo "Failed to enter build directory"; exit 1; }

  if [[ ${#test_names[@]} -eq 0 ]]; then
    # No test names specified, run all tests
    color_print $CYAN "RUNNING ALL TESTS..."
    for test_executable in tests/*/*.test; do
      if [[ -x "$test_executable" ]]; then
        color_print $BLUE "RUNNING TEST FILE ${test_executable}" && "$test_executable" || { color_print $RED "Execution of ${test_executable} failed"; popd; exit 1; }
      else
        color_print $YELLOW "Test executable ${test_executable} not found or is not executable"
      fi
    done
  else
    # Specific test names provided, run only those tests
    color_print $CYAN "RUNNING SPECIFIED TESTS..."
    for test_name in "${test_names[@]}"; do
      test_executable="tests/${test_name}/${test_name}.test"
      if [[ -x "$test_executable" ]]; then
        color_print $BLUE "RUNNING TEST FILE ${test_executable}" && "$test_executable" || { color_print $RED "Execution of ${test_executable} failed"; popd; exit 1; }
      else
        color_print $YELLOW "Test executable ${test_executable} not found or is not executable"
      fi
    done
  fi

  popd
}

# Function to handle all operations based on flags
execute_operations() {
  if $flag_run_cmake; then
    run_cmake
  fi

  if $flag_run_make; then
    run_make
  fi

  if $flag_run_testbed; then
    run_testbed
  fi
}

while [[ $# -gt 0 ]]; do
  case $1 in
    -c)
      flag_run_cmake=true
      ;;
    -m)
      flag_run_make=true
      ;;
    -r)
      flag_run_cmake=true
      flag_run_make=true
      flag_run_testbed=true
      ;;
    -t)
      cmake_options="$cmake_options -DLOG_TEST=ON"
      cmake_options="$cmake_options -DLOG_FILES=OFF"
      flag_run_cmake=true
      flag_run_make=true
      handle_test_names=true
      ;;
    -nw)
      cmake_options="$cmake_options -DLOG_WARN=OFF"
      ;;
    -ni)
      cmake_options="$cmake_options -DLOG_INFO=OFF"
      ;;
    -nt)
      cmake_options="$cmake_options -DLOG_TRACE=OFF"
      ;;
    -nd)
      cmake_options="$cmake_options -DLOG_DEBUG=OFF"
      ;;
    -nf)
      cmake_options="$cmake_options -DLOG_FILES=OFF"
      ;;
    -n)
      handle_test_names=true
      ;;
    *)
      if $handle_test_names; then
        # Collect test names
        while [[ $# -gt 0 && $1 != -* ]]; do
          test_names+=("$1")
          shift
        done
        handle_test_names=false
      else
        echo "Invalid option: $1" >&2
        exit 1
      fi
      ;;
  esac
  shift
done

# Execute based on flags
execute_operations

# Run tests if specified or if LOG_TEST is enabled
if [[ ${#test_names[@]} -gt 0 || $cmake_options == *-DLOG_TEST=ON* ]]; then
  run_tests
fi

