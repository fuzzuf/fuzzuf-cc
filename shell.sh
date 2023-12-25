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
set -e
script_dir=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)

BASE_CONTAINER=fuzzuf/fuzzuf-cc

docker build -t ${BASE_CONTAINER} -f ${script_dir}/Dockerfile ${script_dir} $1
docker run --rm -it \
    -v $(realpath ${script_dir}):/app \
    -u $(id -u ${USER}):$(id -g ${USER}) \
    -w /app \
    ${BASE_CONTAINER}:latest \
    bash
