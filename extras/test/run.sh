#!/bin/sh
# Compile T9602.cpp on the host against the stubs here and diff the output
# against baseline.txt. Usage: ./run.sh            (test)
#                             ./run.sh --record   (rewrite baseline.txt)
cd "$(dirname "$0")" || exit 1
g++ -std=c++17 -Wall -Wno-unused-function -I. -o t9602_test test_output.cpp || exit 1
./t9602_test > output.txt || exit 1
if [ "$1" = "--record" ]; then cp output.txt baseline.txt; echo "baseline recorded"; exit 0; fi
if diff -u baseline.txt output.txt; then echo "OK: output identical to baseline"; else echo "FAIL: output differs from baseline"; exit 1; fi
