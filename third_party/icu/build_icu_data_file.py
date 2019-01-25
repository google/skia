#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Builds ICU from a clean repository and saves the data file.
'''

import glob
import os
import shutil
import subprocess
import sys
import tempfile

def build_icu(icu_source_repository, destination):
    if sys.platform.startswith('linux'):
        cur_os = 'Linux'
    elif sys.platform.startswith('darwin'):
        cur_os = 'MacOSX'
    else:
        sys.exit(1)

    cwd = os.getcwd()
    assert os.path.exists(icu_source_repository)
    icu_source_repository = os.path.abspath(icu_source_repository)
    configure = icu_source_repository + '/icu4c/source/runConfigureICU'
    assert os.path.exists(configure)
    destination = os.path.abspath(destination)
    tmp = tempfile.mkdtemp()
    os.chdir(tmp)
    icuflags = ['-DU_CHARSET_IS_UTF8=1',
                '-DU_NO_DEFAULT_INCLUDE_UTF_HEADERS=1',
                '-DU_HIDE_OBSOLETE_UTF_OLD_H=1']
    env_copy = os.environ.copy()
    env_copy['CPPFLAGS'] = ' '.join(icuflags)
    subprocess.check_call([configure,
                           cur_os,
                           '--enable-static',
                           '--disable-shared',
                           '--with-data-packaging=archive'],
                          env=env_copy)
    subprocess.check_call(['make', '-j'])
    data_files = glob.glob('data/out/icudt*.dat')
    assert len(data_files) == 1
    shutil.move(data_files[0], destination)
    os.chdir(cwd)
    shutil.rmtree(tmp)

if __name__ == '__main__':
    print '\n'.join('>>>  %r' % a for a in sys.argv)
    build_icu(sys.argv[1], sys.argv[2])
