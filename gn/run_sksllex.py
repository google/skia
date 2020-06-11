#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

sksllex = sys.argv[1]
clangFormat = sys.argv[2]
src = sys.argv[3]
try:
    subprocess.check_output([sksllex, src + "/sksl/lex/sksl.lex", "Lexer",
                             "Token", src + "/sksl/SkSLLexer.h", src +
                             "/sksl/SkSLLexer.cpp"])
    subprocess.check_call(clangFormat + " -i \"" + src + "/sksl/SkSLLexer.h\"",
                          shell=True)
    subprocess.check_call(clangFormat + " -i \"" + src +
                          "/sksl/SkSLLexer.cpp\"", shell=True)
except subprocess.CalledProcessError as err:
    print("### Lexer error:")
    print(err.output)
    exit(1)
