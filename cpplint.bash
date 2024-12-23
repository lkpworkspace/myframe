#!/usr/bin/env bash
readonly PWD0=$(readlink -fn "$(dirname "$0")")

function main() {
  set -e
  cd "$PWD0"
  # shellcheck disable=SC2038
  find . '(' \
    -name "*.c" -or \
    -name "*.cc" -or \
    -name "*.h" -or \
    -name "*.hpp" -or \
    -name "*.c++" -or \
    -name "*.h++" -or \
    -name "*.hh" -or \
    -name "*.cu" -or \
    -name "*.cpp" -or \
    -name "*.hxx" -or \
    -name "*.cxx" -or \
    -name "*.cuh" \
    ')' -and -not -path "./build/*" \
    -and -not -path "./output/*" \
    -and -not -path "./3rd/*" \
    -and -not -path "./myframe/export.h" \
    | xargs python3 ./cpplint.py
}

main "$@"
