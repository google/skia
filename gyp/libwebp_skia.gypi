# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This allows us to build libwebp with a custom config.h file. It is currently
# needed to work around skbug.com/4037, but perhaps we might have another need
# for it in the future.
{
  'include_dirs': [
    '../third_party/libwebp/webp',
  ],
  'defines': [
    'HAVE_CONFIG_H',
    'WEBP_SWAP_16BIT_CSP',
  ],
}
