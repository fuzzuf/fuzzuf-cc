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
from typing import List, Mapping
from argparse import ArgumentParser

from abc import ABCMeta, abstractproperty, abstractmethod

import os.path as path

from ..types import PathOrStr
from ..metadata import MetaData

from ..config import ROOT_DIR
from ..util.file import path_exists_all

class FeatureFactory(metaclass=ABCMeta):
    def init_argparse(cls, parser: ArgumentParser):
        pass
    
    @abstractmethod
    def create(self, argument, features: "List[Feature]") -> "Feature": ...

class Feature:
    def __init__(
            self,
            feature_dir="",
            object_paths: List[PathOrStr]=[],
            llvm_pass_obj_paths: List[PathOrStr]=[],
            llvm_lto_pass_obj_paths: List[PathOrStr]=[],
            compile_envvars: Mapping[str, str]={},
            link_envvars: Mapping[str, str]={},
            headers: List[PathOrStr]=[],
        ) -> None:
        self._feature_dir = feature_dir
        self._object_paths = object_paths
        self._llvm_pass_obj_paths = llvm_pass_obj_paths
        self._llvm_lto_pass_obj_paths = llvm_lto_pass_obj_paths
        self._compile_envvars = compile_envvars
        self._link_envvars = link_envvars
        self._headers = headers

    @property
    def feature_dir(self) -> PathOrStr:
        return self._feature_dir

    @property
    def object_paths(self) -> List[PathOrStr]:
        return self._object_paths

    @property
    def llvm_pass_obj_paths(self) -> List[PathOrStr]:
        return self._llvm_pass_obj_paths

    @property
    def llvm_lto_pass_obj_paths(self) -> List[PathOrStr]:
        return self._llvm_lto_pass_obj_paths

    @property
    def compile_envvars(self) -> Mapping[str, str]:
        return self._compile_envvars

    @property
    def link_envvars(self) -> Mapping[str, str]:
        return self._link_envvars

    @property
    def headers(self) -> List[PathOrStr]:
        return self._headers

    def all_path_exists(self):
        return path_exists_all(
            path.join(ROOT_DIR, self.feature_dir),
            self.object_paths + self.llvm_pass_obj_paths + self.llvm_lto_pass_obj_paths + self.headers
        )

    @abstractproperty
    def metadata(self) -> MetaData: ...
