#!/bin/bash
# fuzzuf-cc
# Copyright (C) 2023 Ricerca Security
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
# 
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.

BINARY_DIR=${1}
INPUT=/dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdin

function die() {
  echo $1 1>&2
  rm -f ${INPUT}
  exit 1
}

### test: stdin
echo test.z3Hs >${INPUT} || die "Unable to create input file"
TEST=stdin ${BINARY_DIR}/features/forkserver/benchmark_client ${BINARY_DIR}/test/features/forkserver/test_put stdin | grep test.z3Hs  || die "Expected output is not produced"

rm -f ${INPUT}
exit 0
