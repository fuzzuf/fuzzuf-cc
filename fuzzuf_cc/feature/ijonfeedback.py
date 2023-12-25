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
import sys

from .feature import Feature, FeatureFactory
from ..metadata import MetaData

class IjonFeedbackFactory(FeatureFactory):
    def create(self, _argument, _features):
        feature = IjonFeedback(
            feature_dir="features/ijon",
            object_paths=["ijon-llvm-rt.o"],
            headers=["ijon.h"]
        )
        feature.mapSizePow2 = 16
        feature.shmEnvVar = "__AFL_SHM_ID"

        if not feature.all_path_exists():
            print("\033[33m[!]\033[0m bitmap module is not built. Please build the missing module.", file=sys.stderr)
            exit(1)
        
        return feature

# TODO: mapSizePow2, shmEnvVar を指定できるようにする / バイナリ側に渡す
class IjonFeedback(Feature):
    mapSizePow2: int
    shmEnvVar: str

    @property
    def metadata(self) -> MetaData:
        return MetaData("ijon", 1, {
            "mapSizePow2": self.mapSizePow2, 
            "shmEnvVar": self.shmEnvVar
        })
