# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Creates a .tar.gz file containing an HTML treemap displaying the codesize.

   Requires docker to be installed.

   Example usage:
   python make_treemap.py $SKIA_ROOT/out/Release/skottie_tool /tmp/size

"""


import os
import subprocess
import sys
import tempfile

DOCKER_IMAGE = 'gcr.io/skia-public/binary-size:v1'
DOCKER_SCRIPT = '/opt/binary_size/src/run_binary_size_analysis.py'

def main():
  input_file = sys.argv[1]
  out_dir = sys.argv[2]

  input_base = os.path.basename(input_file)
  input_dir = os.path.dirname(input_file)
  temp_out = tempfile.mkdtemp('treemap')

  subprocess.check_call(['docker', 'run', '--volume', '%s:/IN' % input_dir,
                         '--volume', '%s:/OUT' % temp_out,
                         DOCKER_IMAGE, DOCKER_SCRIPT,
                         '--library', '/IN/%s' % input_base,
                         '--destdir', '/OUT'])

  subprocess.check_call(['tar', '--directory=%s' % temp_out, '-zcf',
                         '%s/%s_tree.tar.gz' % (out_dir, input_base),
                         '.'])

  # Delete our temporary directory
  subprocess.check_call(['docker', 'run',
                         '--volume', '%s:/OUT' % temp_out,
                         DOCKER_IMAGE, '/bin/sh', '-c', 'rm -rf /OUT/*'])

if __name__ == '__main__':
  main()
