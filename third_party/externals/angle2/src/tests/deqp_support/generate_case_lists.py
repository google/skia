#!/usr/bin/python
#
# Copyright 2015 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_case_lists.py:
#   Helper script for updating the dEQP case list files, stored in the repo.
#   Generally only used when the dEQP config changes, or when we roll dEQP.

import subprocess
import sys
import os
import shutil
import gzip

# TODO(jmadill): other platforms
os_suffix = '.exe'
build_dir = os.path.join('build', 'Debug_x64')

def run_deqp(deqp_exe):
    subprocess.call([deqp_exe, '--deqp-runmode=txt-caselist', '--deqp-gl-context-type=null'])

# This stuff is all hard-coded for now. If we need more versatility we can
# make some options into command line arguments with default values.
script_dir = os.path.dirname(sys.argv[0])
path_to_deqp_exe = os.path.join('..', '..', build_dir)
deqp_data_path = os.path.join('third_party', 'deqp', 'data')

os.chdir(os.path.join(script_dir, '..'))
run_deqp(os.path.join(path_to_deqp_exe, 'angle_deqp_gles2_tests' + os_suffix))
run_deqp(os.path.join(path_to_deqp_exe, 'angle_deqp_gles3_tests' + os_suffix))
run_deqp(os.path.join(path_to_deqp_exe, 'angle_deqp_egl_tests' + os_suffix))

def compress_case_list(case_file):
    with open(os.path.join(deqp_data_path, case_file + '.txt')) as in_handle:
        data = in_handle.read()
        in_handle.close()
        with gzip.open(os.path.join('deqp_support', case_file + '.txt.gz'), 'wb') as out_handle:
            out_handle.write(data)
            out_handle.close()

compress_case_list('dEQP-GLES2-cases')
compress_case_list('dEQP-GLES3-cases')
compress_case_list('dEQP-EGL-cases')
