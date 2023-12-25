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
from typing import Literal

from .feature import Feature, FeatureFactory
from ..metadata import MetaData

from argparse import ArgumentParser

TIMING = ["OptimizerLast", "FullLinkTimeOptimizationLast"]
_TIMING_T = Literal["OptimizerLast", "FullLinkTimeOptimizationLast"]

INSTRUMENT_METHOD = ["HASHED_EDGE", "NODE"]
_INSTRUMENTATION_METHOD_T = Literal["HASHED_EDGE", "NODE"]

UPDATE_METHOD = ["NAIVE", "AVOID_ZERO", "CAP_FF"]
_UPDATE_METHOD_T = Literal["NAIVE", "AVOID_ZERO", "CAP_FF"]

ON_LINK = ["FullLinkTimeOptimizationLast"]

# TODO: mapSizePow2, shmEnvVar を指定できるようにする / バイナリ側に渡す
class BitMapFeedbackFactory(FeatureFactory):
    def __init__(self, 
                 default_load_point: _TIMING_T="OptimizerLast",
                 default_instrumentation_method: _INSTRUMENTATION_METHOD_T="HASHED_EDGE",
                 default_update_method: _UPDATE_METHOD_T="NAIVE") -> None:
        self.default_load_point = default_load_point
        self.default_instrumentation_method = default_instrumentation_method
        self.default_update_method = default_update_method

    def init_argparse(self, parser: ArgumentParser):
        group = parser.add_argument_group("Bitmap Feedback")
        group.add_argument("--bitmap-load-point", choices=TIMING, default=self.default_load_point)
        group.add_argument("--bitmap-instrumentation-method", choices=INSTRUMENT_METHOD, default=self.default_instrumentation_method)
        group.add_argument("--bitmap-update-method", choices=UPDATE_METHOD, default=self.default_update_method)

    def create(self, argument, _features):
        env = {
            "BITMAP_PASS_LOAD_POINT": argument.bitmap_load_point,
            "BITMAP_UPDATE_METHOD": argument.bitmap_update_method,
            "BITMAP_INSTRUMENTATION_METHOD": argument.bitmap_instrumentation_method
        }
        if argument.bitmap_load_point in ON_LINK:
            feature = BitMapFeedback(
                feature_dir="features/bitmap",
                object_paths=["bitmap-llvm-rt.o"],
                llvm_lto_pass_obj_paths=["bitmap-llvm-pass.so"],
                link_envvars=env
            )
        else:
            feature = BitMapFeedback(
                feature_dir="features/bitmap",
                object_paths=["bitmap-llvm-rt.o"],
                llvm_pass_obj_paths=["bitmap-llvm-pass.so"],
                compile_envvars=env
            )
        feature.mapSizePow2 = 16
        feature.shmEnvVar = "__AFL_SHM_ID"
        feature.loadPoint = argument.bitmap_load_point
        feature.instrumentationMethod = argument.bitmap_instrumentation_method
        feature.updateMethod = argument.bitmap_update_method
        
        if not feature.all_path_exists():
            print("\033[33m[!]\033[0m bitmap module is not built. Please build the missing module.", file=sys.stderr)
            exit(1)

        return feature

class BitMapFeedback(Feature):
    mapSizePow2: int
    shmEnvVar: str
    loadPoint: str
    instrumentationMethod: str
    updateMethod: str

    @property
    def metadata(self) -> MetaData:
        return MetaData("bitmap", 2, {
            "mapSizePow2": self.mapSizePow2, 
            "shmEnvVar": self.shmEnvVar,
            "loadPoint": self.loadPoint,
            "instrumentationMethod": self.instrumentationMethod,
            "updateMethod": self.updateMethod
        })
