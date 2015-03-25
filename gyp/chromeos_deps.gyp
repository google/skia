# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# This GYP file stores the dependencies necessary to build Skia on the Chrome OS
# platform. The OS doesn't provide many stable libraries as part of the
# distribution so we have to build a few of them ourselves.

{
  'includes': [
    '../platform_tools/chromeos/gyp/dependencies.gypi',
  ],
}
