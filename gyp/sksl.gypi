# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'include_dirs': [
    '../include',
    '../include/config',
    '../include/core',
    '../include/private',
    '../src/sksl',
  ],
  'sources': [
    '../src/sksl/SkSLCompiler.cpp',
    '../src/sksl/SkSLIRGenerator.cpp',
    '../src/sksl/SkSLParser.cpp',
    '../src/sksl/SkSLSPIRVCodeGenerator.cpp',
    '../src/sksl/SkSLUtil.cpp',
    '../src/sksl/ir/SkSLSymbolTable.cpp',
    '../src/sksl/ir/SkSLType.cpp',
  ],
}
