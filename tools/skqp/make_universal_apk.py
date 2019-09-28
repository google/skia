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
    intermediate build objects will be placed here.
  * If the environment variable SKQP_OUTPUT_DIR is set, the final APK
    will be placed in this directory.
  * If the environment variable SKQP_DEBUG is set, Skia will be compiled
    in debug mode.
'''

import os
import sys

import create_apk
import download_model

def make_apk(opts):
    assert '/' in [os.sep, os.altsep]  # 'a/b' over os.path.join('a', 'b')

    skia_dir = os.path.dirname(__file__) + '/../..'
    create_apk.makedirs(opts.build_dir)
    assets_dir = skia_dir + '/platform_tools/android/apps/skqp/src/main/assets'
    gmkb = assets_dir + '/gmkb'
    resources_path = assets_dir + '/resources'

    with create_apk.RemoveFiles(resources_path, gmkb):  # always clean up
        create_apk.remove(gmkb)
        create_apk.make_symlinked_subdir(gmkb, opts.build_dir)

        create_apk.remove(resources_path)
        os.symlink('../../../../../../../resources', resources_path)

        if os.path.exists(assets_dir + '/files.checksum'):
            download_model.main()
        else:
            sys.stderr.write(
                    '\n* * *  Note: SkQP models are missing!  * * *\n\n')
        create_apk.create_apk(opts)

def main():
    options = create_apk.SkQP_Build_Options()
    if options.error:
        sys.stderr.write(options.error + __doc__)
        sys.exit(1)
    options.write(sys.stdout)
    make_apk(options)

if __name__ == '__main__':
    main()
