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
from argparse import ArgumentParser
from typing import Optional, Mapping, List

from .instrumentor import Instrumentor
from .feature.feature import FeatureFactory

class MetaInstrumentor:
    def __init__(self, name: str, version: str, author: str, base_cc: str) -> None:
        self.instrumentor = Instrumentor(name, version, author, base_cc)
        self.features: Mapping[str, FeatureFactory] = {}

    def create_argparse(self):
        # We should disable implicit abbrev here because flags that should be passed to another application could be misinterpreted
        parser = ArgumentParser(self.instrumentor.name, allow_abbrev=False, add_help=False)
        parser.add_argument_group(self.instrumentor.name)
        parser.add_argument("--help", "-h", action="store_true", help="show this help message and exit")
        parser.add_argument("--features")
        return parser

    def print_help(self, parser: ArgumentParser):
        parser.print_help()
        print()
        self.instrumentor.instrument(["--help"])

    def add_feature(self, name: str, factory: FeatureFactory):
        self.features[name] = factory

    def instrument(self, args: Optional[List[str]]=None):
        parser = self.create_argparse()
        args, remain = parser.parse_known_args(args)
        features = args.features.split(",") if args.features is not None else []

        for feature_key in features:
            if feature_key not in self.features.keys():
                print("[!] Unsupported feature '%s'" % feature_key)
                print("[!] Currently available features: %s" % list(self.features.keys()))
                exit(1)
            self.instrumentor.add_feature(self.features[feature_key])

        if args.help:
            self.print_help(parser)
            exit(0)

        self.instrumentor.instrument(remain)
