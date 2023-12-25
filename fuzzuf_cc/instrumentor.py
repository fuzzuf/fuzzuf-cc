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
import os
import os.path as path
import sys
import tempfile
import json
import subprocess
import secrets
from typing import List, Optional
from argparse import ArgumentParser

from .feature.feature import Feature, FeatureFactory
from .metadata import MetaData
from .config import ROOT_DIR, LLVM_DIR

class Instrumentor:
    def __init__(self, name: str, version: str, author: str, base_cc: str):
        self.name = name
        self.version = version
        self.base_cc = base_cc
        self.feature_factories: List[FeatureFactory] = []

        print(
            "\033[36m{}\033[0m \033[1m{}\033[0m{}".format(
                name, version, f" by {author}" if author else ""), 
            file=sys.stderr
        )

    def init_argparse(self):
        # We should disable implicit abbrev here because flags that should be passed to another application could be misinterpreted
        parser = ArgumentParser(self.name, allow_abbrev=False, add_help=False)
        parser.add_argument("--help", "-h", action="store_true", help="show this help message and exit")
        group = parser.add_mutually_exclusive_group()
        group.add_argument("--disable-lto", action="store_true", help="disable link time optimization")
        group.add_argument("--inst-linking", dest="linking", action="store_const", const=True, default=None, help="enforce to link in the instrumentation")
        group.add_argument("--inst-no-linking", dest="linking", action="store_const", const=False, default=None, help="enforce not to link in the instrumentation")
        for feature in self.feature_factories:
            feature.init_argparse(parser)
        return parser

    def print_help(self, parser: ArgumentParser):
        parser.print_help()
        print()
        subprocess.run([self.base_cc, "--help"])

    @property
    def metadata(self):
        return MetaData("instrumentor", 1, {
            "ccName": "fuzzuf-cc",
            "ccVersion": "0.2.0",
            "name": self.name,
            "version": self.version,
        })

    def add_feature(self, feature_factory: FeatureFactory):
        self.feature_factories.append(feature_factory)

    def instrument(self, args: Optional[List[str]]=None):
        parser = self.init_argparse()
        res, remain = parser.parse_known_args(args)
        features: List[Feature] = []
        for feature_factory in self.feature_factories:
            features.append(feature_factory.create(res, features))

        if res.help:
            self.print_help(parser)
            exit(0)
        
        if len(remain) == 0 or any(flag in remain for flag in ["-v", "--version", "-dumpversion", "-print-search-dirs", "-E", "-print-prog-name=ld"]):
            cc_args = [self.base_cc]
            cc_args.extend(remain)
            subprocess.run(cc_args, shell=False, check=True)
            exit(0)

        enable_lto = not res.disable_lto
        if not enable_lto and any(len(feature.llvm_lto_pass_obj_paths) != 0 for feature in features):
            print("[!] Since Link Time Optimization is disabled, LLVM Pass that loaded on Link Time will not be executed ", file=sys.stderr)

        if res.linking is not None:
            linking = res.linking
        else:
            # detect linking with heuristics
            linking = True
            if (len(remain) == 1 and remain[0] == "-v") or \
                any(flag in remain for flag in ["-c", "-S", "-emit-ast", "-emit-llvm"]):
                linking = False

        put = "a.out"
        if "-o" in remain:
            put_ind = remain.index("-o") + 1
            if len(remain) <= put_ind:
                print("[!] argument to `-o` is missing (expected 1 value)")
                exit(1)
            put = remain[put_ind]
        elif "-c" in remain:
            filepath = list(filter(lambda arg: arg.endswith(".c") or arg.endswith(".cc") or arg.endswith(".cxx") or arg.endswith(".cpp"), remain))
            if len(filepath) != 1:
                print("[!] Input filepath is missing (expected 1 value)")
                exit(1)
            put = path.join("./", path.splitext(path.basename(filepath[0]))[0] + ".o")

        cc_args = [self.base_cc]

        for feature in features:
            for header in feature.headers:
                cc_args.append(f"-include")
                cc_args.append(path.join(ROOT_DIR, feature.feature_dir, header))


        for feature in features:
            for object_path in feature.llvm_pass_obj_paths:
                cc_args.append(f"-fpass-plugin={path.join(ROOT_DIR, feature.feature_dir, object_path)}")

        if enable_lto:
            cc_args.append("-flto=full")
            if linking:
                cc_args.append(f"--ld-path={path.join(LLVM_DIR, 'bin/ld.lld')}")
                for feature in features:
                    for object_path in feature.llvm_lto_pass_obj_paths:
                        cc_args.append(f"-Wl,--load-pass-plugin={path.join(ROOT_DIR, feature.feature_dir, object_path)}")
 
        cc_args.extend(remain)

        if linking:
            for feature in features:
                for object_path in feature.object_paths:
                    cc_args.append(path.join(ROOT_DIR, feature.feature_dir, object_path))

        envvars = os.environ.copy()
        for feature in features:
            # TODO: リンクのみの時を判定し、compile_envvars をつけないようにする
            envvars.update(feature.compile_envvars)
            if linking:
                envvars.update(feature.link_envvars)

        subprocess.run(cc_args, env=envvars, shell=False, check=True)

        with tempfile.NamedTemporaryFile(mode='w') as tmp_file:
            data = MetaData.pack([feature.metadata for feature in features] + [self.metadata], path.basename(put))
            json.dump(data, tmp_file.file)
            tmp_file.flush()

            section_name = f".fuzzing-config-{secrets.token_hex(8)}"

            output = subprocess.check_output(["file", put])
            accept_fail = b"LLVM IR bitcode" in output

            try:
                subprocess.run([
                    "objcopy", "--add-section", f"{section_name}={tmp_file.name}", "--set-section-flags", f"{section_name}=noload,readonly", put
                ], check=True, stderr=subprocess.DEVNULL)
            except:
                if not accept_fail:
                    print(f"[*] cc_args = {cc_args}", file=sys.stderr)
                    raise
                # TODO: LTO のときもセクションを追加できるようにする

        
        print("\033[1m\033[32m[+]\033[0m Instrumented done", file=sys.stderr)
