# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'include_dirs': [
    '<(skia_include_path)/config',
    '<(skia_include_path)/core',
    '<(skia_include_path)/private',
    '<(skia_src_path)/sksl',
  ],
  'sources': [
    '<(skia_src_path)/sksl/SkSLCompiler.cpp',
    '<(skia_src_path)/sksl/SkSLIRGenerator.cpp',
    '<(skia_src_path)/sksl/SkSLParser.cpp',
    '<(skia_src_path)/sksl/SkSLGLSLCodeGenerator.cpp',
    '<(skia_src_path)/sksl/SkSLSPIRVCodeGenerator.cpp',
    '<(skia_src_path)/sksl/SkSLUtil.cpp',
    '<(skia_src_path)/sksl/ir/SkSLSymbolTable.cpp',
    '<(skia_src_path)/sksl/ir/SkSLType.cpp',
  ],
}
