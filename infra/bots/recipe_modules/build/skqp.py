# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DOCKER_IMAGE = 'gcr.io/skia-public/android-skqp:r20_v1'
INNER_BUILD_DIR = '/SRC/skia/infra/skqp'
INNER_BUILD_SCRIPT = './build_apk.sh'

def compile_fn(api, checkout_root, _ignore):
  out_dir = api.vars.cache_dir.join('docker', 'skqp')
  # We want to make sure the directories exist and were created by chrome-bot,
  # because if that isn't the case, docker will make them and they will be
  # owned by root, which causes mysterious failures. To mitigate this risk
  # further, we don't use the same out_dir as everyone else (thus the _ignore)
  # param. Instead, we use a "skqp" subdirectory in the "docker" named_cache.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  # This uses the emscriptem sdk docker image and says "run the
  # build_apk.sh helper script in there". Additionally, it binds two
  # folders: the skia checkout to /SRC and the output directory to /OUT
  # The called helper script will make the compile happen and put the
  # output in the right spot.  The neat thing is that since the Skia checkout
  # (and, by extension, the build script) is not a part of the image, but
  # bound in at runtime, we don't have to re-build the image, except when the
  # toolchain changes.
  cmd = ['docker', 'run', '--rm',
         '--workdir', INNER_BUILD_DIR,
         '--volume', '%s:/SRC' % checkout_root,
         '--volume', '%s:/OUT' % out_dir,
         DOCKER_IMAGE, INNER_BUILD_SCRIPT]
  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Build SKQP apk with Docker',
        cmd=cmd)


SKQP_BUILD_PRODUCTS = [
  '*.apk',
  'whitelist_devices.json',
]


def copy_build_products(api, _ignore, dst):
  out_dir = api.vars.cache_dir.join('docker', 'skqp')
  # We don't use the normal copy_listed_files because it uses
  # shutil.move, which attempts to delete the previous file, which
  # doesn't work because the docker created outputs are read-only and
  # owned by root (aka only docker images). It's likely safe to change
  # the shutil.move in the original script to a non-deleting thing
  # (like copy or copyfile), but there's some subtle behavior differences
  # especially with directories, that kjlubick felt it best not to risk it.
  api.python.inline(
      name='copy apk output',
      program='''import errno
import glob
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]
build_products = %s

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
    print 'Copying build product %%s to %%s' %% (f, dst_path)
    # Because Docker usually has some strange permissions (like root
    # ownership), we'd rather not keep those around. copyfile doesn't
    # keep the metadata around, so that helps us.
    shutil.copyfile(f, dst_path)
''' % str(SKQP_BUILD_PRODUCTS),
      args=[out_dir, dst],
      infra_step=True)
