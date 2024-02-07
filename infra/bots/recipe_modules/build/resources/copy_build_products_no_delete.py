# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import errno
import glob
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]
build_products = sys.argv[3].split(',')

try:
  os.makedirs(dst)
except OSError as e:
  if e.errno != errno.EEXIST:
    raise

for pattern in build_products:
  path = os.path.join(src, pattern)
  for f in glob.glob(path):
    dst_path = os.path.join(dst, os.path.relpath(f, src))
    if not os.path.isdir(os.path.dirname(dst_path)):
      os.makedirs(os.path.dirname(dst_path))
    print('Copying build product %s to %s' % (f, dst_path))
    # Because Docker usually has some strange permissions (like root
    # ownership), we'd rather not keep those around. copyfile doesn't
    # keep the metadata around, so that helps us.
    shutil.copyfile(f, dst_path)
