#!/bin/bash -eu
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

# NOTE: "*.cfg.expected" files are generated by the fuzzuf-cc cfg-exporter feature, and are manually verified.
# Future updates may change the shape and format of CFGs.

TARGET_BINARY=${1}
ACTUAL_CFG_FILE="${TARGET_BINARY}.cfg.actual"
EXPECTED_CFG_FILE=${2}

objcopy -O binary --only-section .cfg-* "${TARGET_BINARY}" "${ACTUAL_CFG_FILE}"

if cmp -s "${ACTUAL_CFG_FILE}" "${EXPECTED_CFG_FILE}"; then
  echo "PASS"
  exit 0
else
  echo "FAIL"
  exit 1
fi
