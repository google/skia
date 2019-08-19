#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
This script can be run with no arguments, in which case it will produce an
APK with native libraries for all four architectures: arm, arm64, x86, and
x64.  You can instead list the architectures you want as arguments to this
script.  For example:

    python create_apk.py arm x86

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
import tempfile

import skqp_gn_args

def print_cmd(cmd, o):
    m = re.compile('[^A-Za-z0-9_./-]')
    o.write('+ ')
    for c in cmd:
        if m.search(c) is not None:
            o.write(repr(c) + ' ')
        else:
            o.write(c + ' ')
    o.write('\n')
    o.flush()

def check_call(cmd, **kwargs):
    print_cmd(cmd, sys.stdout)
    return subprocess.check_call(cmd, **kwargs)

def find_name(searchpath, filename):
    for dirpath, _, filenames in os.walk(searchpath):
        if filename in filenames:
            yield os.path.join(dirpath, filename)

def check_ninja():
    with open(os.devnull, 'w') as devnull:
        return 0 == subprocess.call(['ninja', '--version'],
                                    stdout=devnull, stderr=devnull)

def remove(p):
    if not os.path.islink(p) and os.path.isdir(p):
        shutil.rmtree(p)
    elif os.path.lexists(p):
        os.remove(p)
    assert not os.path.exists(p)

def makedirs(dst):
    if not os.path.exists(dst):
        os.makedirs(dst)

class RemoveFiles:
    def __init__(self, *args):
        self.args = args
    def __enter__(self): pass
    def __exit__(self, a, b, c):
        for a in self.args:
            remove(a)

class ChDir:
    def __init__(self, d):
        self.orig = os.getcwd()
        os.chdir(d)
    def __enter__(self): pass
    def __exit__(self, a, b, c):
        os.chdir(self.orig)

def make_symlinked_subdir(target, working_dir):
    newdir = os.path.join(working_dir, os.path.basename(target))
    makedirs(newdir)
    os.symlink(os.path.relpath(newdir, os.path.dirname(target)), target)


skia_to_android_arch_name_map = {'arm'  : 'armeabi-v7a',
                                 'arm64': 'arm64-v8a'  ,
                                 'x86'  : 'x86'        ,
                                 'x64'  : 'x86_64'     }

def create_apk_impl(architectures,
                    android_ndk,
                    android_home,
                    build_dir,
                    final_output_dir,
                    debug):
    assert '/' in [os.sep, os.altsep]  # 'a/b' over os.path.join('a', 'b')
    assert check_ninja()
    assert os.path.exists(android_ndk)
    assert os.path.exists(android_home)
    assert os.path.exists('bin/gn')  # Did you `tools/git-syc-deps`?
    assert architectures
    assert all(arch in skia_to_android_arch_name_map
               for arch in architectures)

    for d in [build_dir, final_output_dir]:
        makedirs(d)

    apps_dir = 'platform_tools/android/apps'
    app = 'skqp'
    lib = 'lib%s_app.so' % app

    # These are the locations in the tree where the gradle needs or will create
    # not-checked-in files.  Treat them specially to keep the tree clean.
    remove(build_dir + '/libs')
    build_paths = [apps_dir + '/.gradle',
                   apps_dir + '/' + app + '/build',
                   apps_dir + '/' + app + '/src/main/libs']
    for path in build_paths:
        remove(path)
        try:
            make_symlinked_subdir(path, build_dir)
        except OSError:
            sys.stderr.write('failed to create symlink "%s"\n' % path)
            pass

    lib_dir = '%s/%s/src/main/libs' % (apps_dir, app)
    apk_build_dir = '%s/%s/build/outputs/apk' % (apps_dir, app)
    for d in [lib_dir, apk_build_dir]:
        shutil.rmtree(d, True)  # force rebuild

    with RemoveFiles(*build_paths):
        for arch in architectures:
            build = os.path.join(build_dir, arch)
            gn_args = skqp_gn_args.GetGNArgs(arch, android_ndk, debug, 26)
            args = ' '.join('%s=%s' % (k,v) for k, v in gn_args.items())
            check_call(['bin/gn', 'gen', build, '--args=' + args])
            check_call(['ninja', '-C', build, lib])
            dst = '%s/%s' % (lib_dir, skia_to_android_arch_name_map[arch])
            makedirs(dst)
            shutil.copy(os.path.join(build, lib), dst)

        env_copy = os.environ.copy()
        env_copy['ANDROID_HOME'] = android_home
        # Why does gradlew need to be called from this directory?
        check_call(['apps/gradlew', '-p' 'apps/' + app,
                    '-P', 'suppressNativeBuild',
                    ':%s:assembleUniversalDebug' % app],
                    env=env_copy, cwd='platform_tools/android')

        apk_name = app + "-universal-debug.apk"

        apk_list = list(find_name(apk_build_dir, apk_name))
        assert len(apk_list) == 1

        out = os.path.join(final_output_dir, apk_name)
        shutil.move(apk_list[0], out)
        sys.stdout.write(out + '\n')

    arches = '_'.join(sorted(architectures))
    copy = os.path.join(final_output_dir, "%s-%s-debug.apk" % (app, arches))
    shutil.copyfile(out, copy)
    sys.stdout.write(copy + '\n')

    sys.stdout.write('* * * COMPLETE * * *\n\n')


def create_apk(architectures,
               android_ndk,
               android_home,
               build_dir,
               final_output_dir,
               debug,
               skia_dir):
    assert os.path.exists(skia_dir)
    with ChDir(skia_dir):
        create_apk_impl(architectures,
                        os.path.abspath(android_ndk),
                        os.path.abspath(android_home),
                        os.path.abspath(build_dir),
                        os.path.abspath(final_output_dir),
                        debug)

def main():
    def error(s):
        sys.stderr.write(s + __doc__)
        sys.exit(1)
    if not check_ninja():
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
    skia_dir = os.path.dirname(__file__) + '/../..'
    default_build = skia_dir + '/out/skqp'
    build_dir = os.environ.get('SKQP_BUILD_DIR', default_build)
    final_output_dir = os.environ.get('SKQP_OUTPUT_DIR', default_build)
    debug = bool(os.environ.get('SKQP_DEBUG', ''))

    for k, v in [('ANDROID_NDK', android_ndk),
                 ('ANDROID_HOME', android_home),
                 ('skia root directory', skia_dir),
                 ('SKQP_OUTPUT_DIR', final_output_dir),
                 ('SKQP_BUILD_DIR', build_dir),
                 ('Architectures', architectures)]:
        sys.stdout.write('%s = %r\n' % (k, v))
    sys.stdout.flush()
    create_apk(architectures,
               android_ndk,
               android_home,
               build_dir,
               final_output_dir,
               debug,
               skia_dir)

if __name__ == '__main__':
    main()

