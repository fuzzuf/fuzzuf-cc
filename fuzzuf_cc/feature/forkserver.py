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

class ForkServerSpecification:
    ### protocolbuffer的なものからいい感じにプロトコル仕様を読み取る
    @property
    def version(self):
        return "0.1.0"

# TODO: channelRecv, channelSend を指定できるようにする / バイナリ側に渡す
class ForkServerFactory(FeatureFactory):
    def create(self, _argument, _features):
        feature = ForkServer(
            feature_dir="features/forkserver",
            object_paths=["forkserver.o"],
        )
        feature.specification = ForkServerSpecification()
        feature.channelRecv = 196
        feature.channelSend = 197
        
        if not feature.all_path_exists():
            print("\033[33m[!]\033[0m ForkServer module is not built. Please build the missing module.", file=sys.stderr)
            exit(1)

        return feature

class ForkServer(Feature):
    specification: ForkServerSpecification
    channelRecv: int
    channelSend: int

    @property
    def metadata(self):
        return MetaData("forkserver", 1, {
            "version": self.specification.version,
            "channelRecv": self.channelRecv, 
            "channelSend": self.channelSend
        })
