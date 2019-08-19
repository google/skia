#! /usr/bin/env python
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
This script can be run with no arguments, in which case it will produce an
APK with native libraries for all four architectures: arm, arm64, x86, and
x64.  You can instead list the architectures you want as arguments to this
script.  For example:

    python make_universal_apk.py arm x86

The environment variables ANDROID_NDK and ANDROID_HOME must be set to the
locations of the Android NDK and SDK.

Additionally, `ninja` should be in your path.

It assumes that the source tree is in the desired state, e.g. by having
run 'python tools/git-sync-deps' in the root of the skia checkout.

Also:
  * If the environment variable SKQP_BUILD_DIR is set, many of the
    intermediate build objects will be places here.
  * If the environment variable SKQP_OUTPUT_DIR is set, the final APK
    will be placed in this directory.
  * If the environment variable SKQP_DEBUG is set, Skia will be compiled
    in debug mode.
'''

import os
import glob
import re
import subprocess
import sys
import shutil

import create_apk

skia_to_android_arch_name_map = {'arm'  : 'armeabi-v7a',
                                 'arm64': 'arm64-v8a'  ,
                                 'x86'  : 'x86'        ,
                                 'x64'  : 'x86_64'     }
def make_apk(architectures,
             android_ndk,
             android_home,
             build_dir,
             final_output_dir,
             debug,
             skia_dir):
    assert '/' in [os.sep, os.altsep]  # 'a/b' over os.path.join('a', 'b')
    assert os.path.exists(skia_dir)

    create_apk.makedirs(build_dir)
    assets_dir = skia_dir + '/platform_tools/android/apps/skqp/src/main/assets'
    gmkb = assets_dir + '/gmkb'
    resources_path = assets_dir + '/resources'

    with create_apk.RemoveFiles(resources_path, gmkb):  # alwyas clean up
        create_apk.remove(gmkb)
        create_apk.make_symlinked_subdir(gmkb, build_dir)

        create_apk.remove(resources_path)
        os.symlink('../../../../../../../resources', resources_path)

        if os.path.exists(assets_dir + '/files.checksum'):
            create_apk.check_call([sys.executable, skia_dir + '/tools/skqp/download_model'])
        else:
            sys.stderr.write(
                    '\n* * *  Note: SkQP models are missing!  * * *\n\n')

        create_apk.create_apk(architectures,
                              android_ndk,
                              android_home,
                              build_dir,
                              final_output_dir,
                              debug,
                              skia_dir)

def main():
    def error(s):
        sys.stderr.write(s + __doc__)
        sys.exit(1)
    if not create_apk.check_ninja():
        error('`ninja` is not in the path.\n')
    for var in ['ANDROID_NDK', 'ANDROID_HOME']:
        if not os.path.exists(os.environ.get(var, '')):
            error('Environment variable `%s` is not set.\n' % var)
    architectures = sys.argv[1:]
    for arg in sys.argv[1:]:
        if arg not in skia_to_android_arch_name_map:
            error('Argument %r is not in %r\n' %
                  (arg, skia_to_android_arch_name_map.keys()))
    if not architectures:
        architectures = skia_to_android_arch_name_map.keys()
    skia_dir = os.path.abspath(
            os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))
    default_build = os.path.join(skia_dir, 'out', 'skqp')
    build_dir = os.path.abspath(os.environ.get('SKQP_BUILD_DIR', default_build))
    final_output_dir = os.path.abspath(
            os.environ.get('SKQP_OUTPUT_DIR', default_build))
    debug = bool(os.environ.get('SKQP_DEBUG', ''))
    android_ndk = os.path.abspath(os.environ['ANDROID_NDK'])
    android_home = os.path.abspath(os.environ['ANDROID_HOME'])

    for k, v in [('ANDROID_NDK', android_ndk),
                 ('ANDROID_HOME', android_home),
                 ('skia root directory', skia_dir),
                 ('SKQP_OUTPUT_DIR', final_output_dir),
                 ('SKQP_BUILD_DIR', build_dir),
                 ('Architectures', architectures)]:
        sys.stdout.write('%s = %r\n' % (k, v))
    sys.stdout.flush()
    make_apk(architectures,
             android_ndk,
             android_home,
             build_dir,
             final_output_dir,
             debug,
             skia_dir)

if __name__ == '__main__':
    main()

