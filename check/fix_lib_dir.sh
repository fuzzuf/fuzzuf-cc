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

function die {
  echo ${1}
  exit 1
}

echo mkdir -p ${2}
mkdir -p ${2} || die "Unable to create destination directory"
echo ${2}
cd ${2}
rm -rf fuzzuf_cc fuzzuf-afl-cc fuzzuf-afl-c++ fuzzuf-clang
echo cp -rpdf ${1}/fuzzuf_cc ./
cp -rpdf ${1}/fuzzuf_cc ./ || die "Unable to copy fuzzuf_cc module"
echo cp -rpdf ./_config.py ./fuzzuf_cc/config.py
cp -rpdf ./_config.py ./fuzzuf_cc/config.py || die "Unable to copy fuzzuf_cc config"
echo cp -rpdf ${1}/fuzzuf-cc ./
cp -rpdf ${1}/fuzzuf-cc ./ || die "Unable to copy fuzzuf-cc"
echo cp -rpdf ${1}/fuzzuf-c++ ./
cp -rpdf ${1}/fuzzuf-c++ ./ || die "Unable to copy fuzzuf-c++"
echo cp -rpdf ${1}/fuzzuf-afl-cc ./
cp -rpdf ${1}/fuzzuf-afl-cc ./ || die "Unable to copy fuzzuf-afl-cc"
echo cp -rpdf ${1}/fuzzuf-afl-c++ ./
cp -rpdf ${1}/fuzzuf-afl-c++ ./ || die "Unable to copy fuzzuf-afl-c++"
echo cp -rpdf ${1}/fuzzuf-clang ./
cp -rpdf ${1}/fuzzuf-clang ./ || die "Unable to copy fuzzuf-clang"
echo cp -rpdf ${1}/fuzzuf-ijon-cc ./
cp -rpdf ${1}/fuzzuf-ijon-cc ./ || die "Unable to copy fuzzuf-ijon-cc"
echo mkdir -p fuzzuf_cc/features/forkserver
mkdir -p fuzzuf_cc/features/forkserver || die "Unable to create runtime directory"
echo cp -rpdf ${3} fuzzuf_cc/features/forkserver/forkserver.o
cp -rpdf ${3} fuzzuf_cc/features/forkserver/forkserver.o || die "Unable to copy runtime"
echo mkdir -p fuzzuf_cc/features/ijon
mkdir -p fuzzuf_cc/features/ijon || die "Unable to create IJON runtime directory"
echo cp -rpdf ${4} fuzzuf_cc/features/ijon/ijon-llvm-rt.o
cp -rpdf ${4} fuzzuf_cc/features/ijon/ijon-llvm-rt.o || die "Unable to copy IJON runtime"
echo cp -rpdf ${1}/features/ijon/ijon.h fuzzuf_cc/features/ijon/ijon.h
cp -rpdf ${1}/features/ijon/ijon.h fuzzuf_cc/features/ijon/ijon.h || die "Unable to copy IJON header"
echo mkdir -p fuzzuf_cc/features/bitmap
mkdir -p fuzzuf_cc/features/bitmap || die "Unable to create bitmap runtime directory"
echo cp -rpdf ${5} fuzzuf_cc/features/bitmap/bitmap-llvm-rt.o
cp -rpdf ${5} fuzzuf_cc/features/bitmap/bitmap-llvm-rt.o || die "Unable to copy bitmap runtime"
echo cp -rpdf ${6} fuzzuf_cc/features/bitmap/bitmap-llvm-pass.so
cp -rpdf ${6} fuzzuf_cc/features/bitmap/bitmap-llvm-pass.so || die "Unable to copy bitmap pass object"
echo mkdir -p fuzzuf_cc/features/bb-id-assigner
mkdir -p fuzzuf_cc/features/bb-id-assigner || die "Unable to create bb-id-assigner directory"
echo cp -rpdf ${7} fuzzuf_cc/features/bb-id-assigner/bb-id-assigner-pass.so
cp -rpdf ${7} fuzzuf_cc/features/bb-id-assigner/bb-id-assigner-pass.so || die "Unable to copy bb-id-assigner pass object"
echo mkdir -p fuzzuf_cc/features/cfg-exporter
mkdir -p fuzzuf_cc/features/cfg-exporter || die "Unable to create cfg-exporter directory"
echo cp -rpdf ${8} fuzzuf_cc/features/cfg-exporter/cfg-exporter-pass.so
cp -rpdf ${8} fuzzuf_cc/features/cfg-exporter/cfg-exporter-pass.so || die "Unable to copy cfg-exporter pass object"

exit 0
