# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# include this gypi to include all the golden master slides.
{
  'include_dirs': [
    '../gm',
    # include dirs needed by particular GMs
    '../src/utils/debugger',
    '../src/images',
    '../src/lazy',
  ],
  'conditions': [
    # If we're building SampleApp on the bots, no need to link in the GM slides.
    # We're not going to run it; we're only making sure it still builds.
    # It'd be nice to do this in SampleApp.gypi, but I can't find a way to make it work.
    [ 'not ("<(_target_name)" == "SampleApp" and skia_is_bot)', {
      'sources': [
        '<!@(python find.py ../gm "*.c*")',

        # Files needed by particular GMs
        '../src/gpu/batches/GrTestBatch.h',
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
      ],
    }],
  ],
}
