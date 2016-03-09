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
      'type': 'none',
      'variables': {
        'base_dir%': '../third_party/libmicrohttpd',
        'out_dir%': '<(INTERMEDIATE_DIR)/build',
        'src_dir%': '../third_party/externals/microhttpd',
      },
      'direct_dependent_settings': {
        'include_dirs': [
          '<(src_dir)/src/include',
        ],
        # Link the built library to dependents.
        'libraries': [
          '<(PRODUCT_DIR)/libmicrohttpd.a',
        ],
      },
      'actions': [
        {
          'action_name': 'configure_and_build',
          'inputs': [
            '<(base_dir)/build.py',
            '<(src_dir)/.git/HEAD', # This does not support local changes, but does support DEPS.
          ],
          'outputs': [ '<(PRODUCT_DIR)/libmicrohttpd.a' ],
          'action': [
            'python',
            '<(base_dir)/build.py', 
            '--src', '<(src_dir)',
            '--out', '<(out_dir)',
            '--dst', '<(PRODUCT_DIR)',
          ],
        },
      ],
    },
  ],
}
