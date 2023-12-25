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
from typing import Literal, Optional

from .feature import Feature, FeatureFactory
from ..metadata import MetaData

from argparse import ArgumentParser

# TODO: mapSizePow2 を指定できるようにする / バイナリ側に渡す

NODE_SELECTION_STRATEGY = ["ALL", "NO_MULTIPLE_PRED"]
_T_NODE_SELECTION_STRATEGY = Literal["ALL", "NO_MULTIPLE_PRED"]

ID_GENERATION_STRATEGY = ["RANDOM", "SEQUENTIAL"]
_T_ID_GENERATION_STRATEGY = Literal["RANDOM", "SEQUENTIAL"]

class BBIDAssignerFactory(FeatureFactory):
    def __init__(self,
                 default_path_splitting: bool=False,
                 default_node_selection_strategy: _T_NODE_SELECTION_STRATEGY="ALL",
                 default_id_generation_strategy: _T_ID_GENERATION_STRATEGY="RANDOM",
                 default_map_size=65536):
        self.default_path_splitting = default_path_splitting
        self.default_node_selection_strategy = default_node_selection_strategy
        self.default_id_generation_strategy = default_id_generation_strategy
        self.default_map_size = default_map_size

    def init_argparse(self, parser: ArgumentParser):
        group = parser.add_argument_group("BB ID Assigner")
        splitting = group.add_mutually_exclusive_group()
        splitting.add_argument("--bb-id-assigner-path-splitting", dest="path_splitting", action="store_true", default=self.default_path_splitting)
        splitting.add_argument("--bb-id-assigner-no-path-splitting", dest="path_splitting", action="store_false", default=self.default_path_splitting)

        group.add_argument("--bb-id-assigner-node-selection-strategy", choices=NODE_SELECTION_STRATEGY, default=self.default_node_selection_strategy)
        group.add_argument("--bb-id-assigner-id-generation-strategy", choices=ID_GENERATION_STRATEGY, default=self.default_id_generation_strategy)
        group.add_argument("--bb-id-assigner-map-size", type=int, default=self.default_map_size)

    def create(self, argument, _features):
        envs = {}

        if argument.path_splitting:
            envs["BB_ID_ASSIGNER_DO_PATH_SPLITTING"] = ""

        envs["BB_ID_ASSIGNER_NODE_SELECTION_STRATEGY"] = argument.bb_id_assigner_node_selection_strategy
        envs["BB_ID_ASSIGNER_ID_GENERATION_STRATEGY"] = argument.bb_id_assigner_id_generation_strategy
        if argument.bb_id_assigner_id_generation_strategy == "RANDOM":
            envs["BB_ID_ASSIGNER_MAP_SIZE"] = str(argument.bb_id_assigner_map_size)
            map_size = argument.bb_id_assigner_map_size
        else:
            map_size = None

        feature = BBIDAssigner(
            feature_dir="features/bb-id-assigner",
            llvm_pass_obj_paths=["bb-id-assigner-pass.so"],
            llvm_lto_pass_obj_paths=["bb-id-assigner-pass.so"],
            compile_envvars=envs,
            link_envvars=envs
        )
        feature.nodeSelectionStrategy = argument.bb_id_assigner_node_selection_strategy
        feature.idGenerationStrategy = argument.bb_id_assigner_id_generation_strategy
        feature.mapSize = map_size

        if not feature.all_path_exists():
            print("\033[33m[!]\033[0m BBIDAssigner module is not built. Please build the missing module.", file=sys.stderr)
            exit(1)

        return feature
        
class BBIDAssigner(Feature):
    nodeSelectionStrategy: str
    idGenerationStrategy: str
    mapSize: Optional[int]
    @property
    def metadata(self) -> MetaData:
        return MetaData("bb-id-assigner", 2, {
            "nodeGenerationStrategy": self.nodeSelectionStrategy,
            "idGenerationStrategy": self.idGenerationStrategy,
            "mapSize": self.mapSize
        })
