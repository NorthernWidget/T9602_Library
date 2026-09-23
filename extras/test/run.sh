#!/bin/sh
# Host-side harness: compile src/T9602.cpp against the NW_Core stubs and diff
# the output against baseline.txt. Usage: ./run.sh [--record]
cd "$(dirname "$0")" || exit 1
NW_CORE="${NW_CORE:-../../../NW_Core}"
exec "$NW_CORE/extras/test/run_library.sh" t9602_test "$1"
