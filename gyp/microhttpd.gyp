# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# A simple gyp file to generate microhttpd for internal purposes
# most of the work(configure and make) is performed in a python script
{
  'targets': [
    {
      'target_name': 'microhttpd',
      'type': 'static_library',
      'variables': {
        'base_dir%': '../third_party/libmicrohttpd',
        'src_dir%': '../third_party/externals/microhttpd',
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '<(src_dir)/src/include',
        ],
        # for reasons I can't quite fathom, we need the below line to trigger
        # a link
        'libraries': [
          'libmicrohttpd.a',
        ],
      },
      'actions': [
        {
          'action_name': 'configure_and_build',
          'inputs': [
            '<(PRODUCT_DIR)/',
          ],
          'outputs': [ '<(PRODUCT_DIR)/libmicrohttpd.a' ],
          'action': [
            'python',
            '<(base_dir)/build.py', 
            '--src', '<(src_dir)',
            '--dst', '<(PRODUCT_DIR)',
          ],
        },
      ],
    },
  ],
}
