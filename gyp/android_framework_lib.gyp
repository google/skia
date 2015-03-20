# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'libskia',
      'type': 'shared_library',
      'dependencies': [
        'codec.gyp:codec',
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'opts.gyp:opts',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'utils.gyp:utils',
      ],
    },
  ],
}
