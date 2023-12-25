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

ON_LINK = ["FullLinkTimeOptimizationLast"]

class CFGExporterFactory(FeatureFactory):
    def __init__(self, default_load_point: _TIMING_T="OptimizerLast") -> None:
        self.default_load_point = default_load_point

    def init_argparse(self, parser: ArgumentParser):
        group = parser.add_argument_group("CFG Exporter")
        group.add_argument("--cfg-exporter-load-point", choices=TIMING, default=self.default_load_point)

    def create(self, argument, _features):
        env = {
            "CFG_EXPORTER_LOAD_POINT": argument.cfg_exporter_load_point
        }
        if argument.cfg_exporter_load_point in ON_LINK:
            feature = CFGExporter(
                feature_dir="features/cfg-exporter",
                llvm_lto_pass_obj_paths=["cfg-exporter-pass.so"],
                link_envvars=env
            )
        else:
            feature = CFGExporter(
                feature_dir="features/cfg-exporter",
                llvm_pass_obj_paths=["cfg-exporter-pass.so"],
                compile_envvars=env
            )

        feature.loadPoint = argument.cfg_exporter_load_point
        
        if not feature.all_path_exists():
            print("\033[33m[!]\033[0m CFGExporter module is not built. Please build the missing module.", file=sys.stderr)
            exit(1)

        return feature

class CFGExporter(Feature):
    loadPoint: str

    @property
    def metadata(self) -> MetaData:
        return MetaData("cfg-exporter", 2, {
            # TODO: add section name
            "loadPoint": self.loadPoint
        })
