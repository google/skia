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

We also assume that the 'resources' directory has been copied to
'platform_tools/android/apps/skqp/src/main/assets', and the
'tools/skqp/download_model' script has been run.

Also:
  * If the environment variable SKQP_BUILD_DIR is set, many of the
    intermediate build objects will be placed here.
  * If the environment variable SKQP_OUTPUT_DIR is set, the final APK
    will be placed in this directory.
  * If the environment variable SKQP_DEBUG is set, Skia will be compiled
    in debug mode.
'''

import os
import re
import subprocess
import sys
import shutil
import time

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
        return subprocess.call(['ninja', '--version'],
                               stdout=devnull, stderr=devnull) == 0

def remove(p):
    if not os.path.islink(p) and os.path.isdir(p):
        shutil.rmtree(p)
    elif os.path.lexists(p):
        os.remove(p)
    assert not os.path.exists(p)

def makedirs(dst):
    if not os.path.exists(dst):
        os.makedirs(dst)

class RemoveFiles(object):
    def __init__(self, *args):
        self.args = args
    def __enter__(self):
        pass
    def __exit__(self, a, b, c):
        for arg in self.args:
            remove(arg)

class ChDir(object):
    def __init__(self, d):
        self.orig = os.getcwd()
        os.chdir(d)
    def __enter__(self):
        pass
    def __exit__(self, a, b, c):
        os.chdir(self.orig)

def make_symlinked_subdir(target, working_dir):
    newdir = os.path.join(working_dir, os.path.basename(target))
    makedirs(newdir)
    os.symlink(os.path.relpath(newdir, os.path.dirname(target)), target)

def accept_android_license(android_home):
    proc = subprocess.Popen(
            [android_home + '/tools/bin/sdkmanager', '--licenses'],
            stdin=subprocess.PIPE)
    while proc.poll() is None:
        proc.stdin.write('y\n')
        time.sleep(1)

# pylint: disable=bad-whitespace
skia_to_android_arch_name_map = {'arm'  : 'armeabi-v7a',
                                 'arm64': 'arm64-v8a'  ,
                                 'x86'  : 'x86'        ,
                                 'x64'  : 'x86_64'     }

def create_apk_impl(opts):
    build_dir, final_output_dir = opts.build_dir, opts.final_output_dir

    assert os.path.exists('bin/gn')  # Did you `tools/git-syc-deps`?

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

    lib_dir = '%s/%s/src/main/libs' % (apps_dir, app)
    apk_build_dir = '%s/%s/build/outputs/apk' % (apps_dir, app)
    for d in [lib_dir, apk_build_dir]:
        shutil.rmtree(d, True)  # force rebuild

    with RemoveFiles(*build_paths):
        for arch in opts.architectures:
            build = os.path.join(build_dir, arch)
            gn_args = opts.gn_args(arch)
            args = ' '.join('%s=%s' % (k, v) for k, v in gn_args.items())
            check_call(['bin/gn', 'gen', build, '--args=' + args])
            check_call(['ninja', '-C', build, lib])
            dst = '%s/%s' % (lib_dir, skia_to_android_arch_name_map[arch])
            makedirs(dst)
            shutil.copy(os.path.join(build, lib), dst)

        accept_android_license(opts.android_home)
        env_copy = os.environ.copy()
        env_copy['ANDROID_HOME'] = opts.android_home
        env_copy['ANDROID_NDK_HOME'] = opts.android_ndk
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

    arches = '_'.join(sorted(opts.architectures))
    copy = os.path.join(final_output_dir, "%s-%s-debug.apk" % (app, arches))
    shutil.copyfile(out, copy)
    sys.stdout.write(copy + '\n')

    sys.stdout.write('* * * COMPLETE * * *\n\n')


def create_apk(opts):
    skia_dir = os.path.abspath(os.path.dirname(__file__) + '/../..')
    assert os.path.exists(skia_dir)
    with ChDir(skia_dir):
        create_apk_impl(opts)

class SkQP_Build_Options(object):
    def __init__(self):
        assert '/' in [os.sep, os.altsep]  # 'a/b' over os.path.join('a', 'b')
        self.error = ''
        if not check_ninja():
            self.error += '`ninja` is not in the path.\n'
        for var in ['ANDROID_NDK', 'ANDROID_HOME']:
            if not os.path.exists(os.environ.get(var, '')):
                self.error += 'Environment variable `%s` is not set.\n' % var
        self.android_ndk = os.path.abspath(os.environ['ANDROID_NDK'])
        self.android_home = os.path.abspath(os.environ['ANDROID_HOME'])
        args = sys.argv[1:]
        for arg in args:
            if arg not in skia_to_android_arch_name_map:
                self.error += ('Argument %r is not in %r\n' %
                               (arg, skia_to_android_arch_name_map.keys()))
        self.architectures = args if args else skia_to_android_arch_name_map.keys()
        default_build = os.path.dirname(__file__) + '/../../out/skqp'
        self.build_dir = os.path.abspath(os.environ.get('SKQP_BUILD_DIR', default_build))
        self.final_output_dir = os.path.abspath(os.environ.get('SKQP_OUTPUT_DIR', default_build))
        self.debug = bool(os.environ.get('SKQP_DEBUG', ''))

    def gn_args(self, arch):
        return skqp_gn_args.GetGNArgs(arch, self.android_ndk, self.debug, 26)

    def write(self, o):
        for k, v in [('ANDROID_NDK', self.android_ndk),
                     ('ANDROID_HOME', self.android_home),
                     ('SKQP_OUTPUT_DIR', self.final_output_dir),
                     ('SKQP_BUILD_DIR', self.build_dir),
                     ('SKQP_DEBUG', self.debug),
                     ('Architectures', self.architectures)]:
            o.write('%s = %r\n' % (k, v))
        o.flush()

def main():
    options = SkQP_Build_Options()
    if options.error:
        sys.stderr.write(options.error + __doc__)
        sys.exit(1)
    options.write(sys.stdout)
    create_apk(options)

if __name__ == '__main__':
    main()
