# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build experimental directory.
{
  'targets': [
    {
      'target_name': 'experimental',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
      ],
      'sources': [
        '../experimental/SkSetPoly3To3.cpp',
        '../experimental/SkSetPoly3To3_A.cpp',
        '../experimental/SkSetPoly3To3_D.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../experimental',
        ],
      },
    },
    {
      'target_name': 'multipage_pdf_profiler',
      'type': 'executable',
      'sources': [
        '../experimental/tools/multipage_pdf_profiler.cpp',
        '../experimental/tools/PageCachingDocument.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'pdf.gyp:pdf',
        'tools.gyp:proc_stats',
        'tools.gyp:sk_tool_utils',
      ],
    },
    {
      'target_name': 'skp_to_pdf_md5',
      'type': 'executable',
      'sources': [
        '../experimental/tools/skp_to_pdf_md5.cpp',
        '../experimental/tools/SkDmuxWStream.cpp',
      ],
      'include_dirs': [
        '../src/core',
        '../tools/flags',
      ],
      'dependencies': [
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:sk_tool_utils',
      ],
    },
  ],
}
