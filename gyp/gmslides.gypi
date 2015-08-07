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
      'sources!': [
        '../gm/annotated_text.cpp',
        '../gm/system_preferences_default.cpp',
        '../gm/techtalk1.cpp',
      ],
    }],
    # TODO: Several GMs are known to cause particular problems on Android, so
    # we disable them on Android.  See http://skbug.com/2326
    [ 'skia_os == "android"', {
      'sources!': [
        # TODO(borenet): Causes assertion failure on Nexus S.
        # See http://skbug.com/705
        '../gm/bitmapcopy.cpp',

        # SOME of the bitmaprect tests are disabled on Android; see
        # ../gm/bitmaprect.cpp

        # We skip GPU tests in this GM; see
        # ../gm/deviceproperties.cpp

        # TODO(bsalomon): Hangs on Xoom and Nexus S. See http://skbug.com/637
        '../gm/drawbitmaprect.cpp',

        # TODO(epoger): Crashes on Nexus 10. See http://skbug.com/2313
        '../gm/imagefilterscropexpand.cpp',

        # TODO(borenet): Causes Nexus S to reboot. See http://skbug.com/665
        '../gm/shadertext.cpp',
        '../gm/shadertext2.cpp',
        '../gm/shadertext3.cpp',

        # TODO(reed): Allocates more memory than Android devices are capable of
        # fulfilling. See http://skbug.com/1978
        '../gm/verylargebitmap.cpp',
      ],
    }],
  ],
}
