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

function die() {
  echo $1 1>&2
  exit 1
}

### test: stdout
TEST=stdout ${BINARY_DIR}/features/forkserver/benchmark_client ${BINARY_DIR}/test/features/forkserver/test_put stdout.91DV || die "Unable to execute benchmark_client"
grep stdout.*91DV /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stdout || die "Expected stdout is not produced"
grep stderr.*91DV /dev/shm/fuzzuf-cc.forkserver.executor_id-1.stderr || die "Expected stderr is not produced"

exit 0
