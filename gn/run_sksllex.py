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
fetchClangFormat = sys.argv[3]
src = sys.argv[4]

exeSuffix = '.exe' if sys.platform.startswith('win') else '';

try:
    subprocess.check_output([sksllex, src + "/sksl/lex/sksl.lex", "Lexer",
                             "Token", src + "/sksl/SkSLLexer.h", src +
                             "/sksl/SkSLLexer.cpp"])

    if not os.path.isfile(clangFormat + exeSuffix):
        subprocess.check_call([sys.executable, fetchClangFormat]);

    subprocess.check_call(clangFormat + " -i \"" + src + "/sksl/SkSLLexer.h\"",
                          shell=True)
    subprocess.check_call(clangFormat + " -i \"" + src +
                          "/sksl/SkSLLexer.cpp\"", shell=True)
except subprocess.CalledProcessError as err:
    print("### Lexer error:")
    print(err.output)
    exit(1)
