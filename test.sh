#!/usr/bin/env bash

EXE=$(pwd)/calculator

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

test_case() {
  local input=$1
  local golden=$2
  printf "Testing input: %s \t... " "${input}"
  if echo "${golden}" | diff <("${EXE}" "${input}" 2>/dev/null ) -  ; then
    echo -e "${GREEN}PASS${NC}"
  else
    echo -e "${RED}FAIL${NC}"
  fi
}

main() {
  test_case "123+123" "246"
  test_case "1+2*3-2" "5"
  test_case "" "0"
  test_case "   " "0"
  test_case "1+    4*7*2*9-89" "416"
  test_case "(1+3)*4" "16"
}

main "$@"
