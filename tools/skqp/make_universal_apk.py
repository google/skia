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
    assert check_ninja()
    assert os.path.exists(android_ndk)
    assert os.path.exists(android_home)
    assert os.path.exists(skia_dir)
    assert os.path.exists(skia_dir + '/bin/gn')  # Did you `tools/git-syc-deps`?
    assert architectures
    assert all(arch in skia_to_android_arch_name_map
               for arch in architectures)

    for d in [build_dir, final_output_dir]:
        if not os.path.exists(d):
            os.makedirs(d)

    os.chdir(skia_dir)
    apps_dir = 'platform_tools/android/apps'

    # These are the locations in the tree where the gradle needs or will create
    # not-checked-in files.  Treat them specially to keep the tree clean.
    aosp_mode = os.path.exists('MODULE_LICENSE_BSD')
    build_paths = [apps_dir + '/.gradle',
                   apps_dir + '/skqp/build',
                   apps_dir + '/skqp/src/main/libs']
    if not aosp_mode:
        build_paths.append(apps_dir + '/skqp/src/main/assets/gmkb')
    remove(build_dir + '/libs')
    for path in build_paths:
        remove(path)
        newdir = os.path.join(build_dir, os.path.basename(path))
        if not os.path.exists(newdir):
            os.makedirs(newdir)
        try:
            os.symlink(os.path.relpath(newdir, os.path.dirname(path)), path)
        except OSError:
            pass

    if not aosp_mode:
        resources_path = apps_dir + '/skqp/src/main/assets/resources'
        remove(resources_path)
        os.symlink('../../../../../../../resources', resources_path)
        build_paths.append(resources_path)

    app = 'skqp'
    lib = 'libskqp_app.so'

    shutil.rmtree(apps_dir + '/%s/src/main/libs' % app, True)

    if not aosp_mode:
        if os.path.exists(apps_dir + '/skqp/src/main/assets/files.checksum'):
            check_call([sys.executable, 'tools/skqp/download_model'])
        else:
            sys.stderr.write(
                    '\n* * *\n\nNote: SkQP models are missing!!!!\n\n* * *\n\n')
    if aosp_mode:
        with open('include/config/SkUserConfig.h') as f:
            user_config = f.readlines()
        with open('include/config/SkUserConfig.h', 'w') as o:
            for line in user_config:
                m = re.match(r'^#define\s+([A-Za-z0-9_]+)(|\s.*)$', line)
                if m:
                    o.write('#ifndef %s\n%s\n#endif\n' % (m.group(1), m.group(0).strip()))
                else:
                    o.write(line)

    for arch in architectures:
        build = os.path.join(build_dir, arch)
        gn_args = [android_ndk, '--arch', arch]
        if debug:
            build += '-debug'
            gn_args += ['--debug']
        check_call([sys.executable, 'tools/skqp/generate_gn_args', build]
                   + gn_args)
        check_call(['bin/gn', 'gen', build])
        check_call(['ninja', '-C', build, lib])
        dst = apps_dir + '/%s/src/main/libs/%s' % (
                app, skia_to_android_arch_name_map[arch])
        if not os.path.isdir(dst):
            os.makedirs(dst)
        shutil.copy(os.path.join(build, lib), dst)

    if aosp_mode:
        subprocess.call('git', 'checkout', 'HEAD', 'include/config/SkUserConfig.h')

    apk_build_dir = apps_dir + '/%s/build/outputs/apk' % app
    shutil.rmtree(apk_build_dir, True)  # force rebuild

    # Why does gradlew need to be called from this directory?
    os.chdir('platform_tools/android')
    env_copy = os.environ.copy()
    env_copy['ANDROID_HOME'] = android_home
    check_call(['apps/gradlew', '-p' 'apps/' + app, '-P', 'suppressNativeBuild',
                ':%s:assembleUniversalDebug' % app], env=env_copy)
    os.chdir(skia_dir)

    apk_name = app + "-universal-debug.apk"

    apk_list = list(find_name(apk_build_dir, apk_name))
    assert len(apk_list) == 1

    out = os.path.join(final_output_dir, apk_name)
    shutil.move(apk_list[0], out)
    sys.stdout.write(out + '\n')

    for path in build_paths:
        remove(path)

    arches = '_'.join(sorted(architectures))
    copy = os.path.join(final_output_dir, "%s-%s-debug.apk" % (app, arches))
    shutil.copyfile(out, copy)
    sys.stdout.write(copy + '\n')

    sys.stdout.write('* * * COMPLETE * * *\n\n')

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

