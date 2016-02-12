#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Script for generating the Android framework's version of Skia from gyp
files.
"""

import argparse
import os
import shutil
import sys
import tempfile

# Find the top of trunk
SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
SKIA_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, os.pardir, os.pardir,
                                         os.pardir))

# Find the directory with our helper files, and add it to the path.
ANDROID_TOOLS = os.path.join(SKIA_DIR, 'platform_tools', 'android')
sys.path.append(ANDROID_TOOLS)

import gyp_gen.android_framework_gyp as android_framework_gyp
import gyp_gen.gypd_parser as gypd_parser
import gyp_gen.generate_user_config as generate_user_config
import gyp_gen.makefile_writer as makefile_writer
import gyp_gen.tool_makefile_writer as tool_makefile_writer
import gyp_gen.vars_dict_lib as vars_dict_lib

# Folder containing all gyp files and generated gypd files.
GYP_FOLDER = 'gyp'


def generate_var_dict(target_dir, target_file, skia_arch_type, have_neon,
                      gyp_source_dir):
  """Create a VarsDict for a particular arch type.

  Each paramater is passed directly to android_framework_gyp.main().

  Args:
    target_dir: Directory containing gyp files.
    target_file: Target gyp file.
    skia_arch_type: Target architecture.
    have_neon: Whether the target should build for neon.
    gyp_source_dir: Directory for gyp source.
  Returns:
    A VarsDict containing the variable definitions determined by gyp.
  """
  result_file = android_framework_gyp.main(target_dir, target_file,
                                           skia_arch_type, have_neon,
                                           gyp_source_dir)
  var_dict = vars_dict_lib.VarsDict()
  gypd_parser.parse_gypd(var_dict, result_file, '.')
  android_framework_gyp.clean_gypd_files(target_dir)
  print '.',
  return var_dict

def main(target_dir=None, require_sk_user_config=False, gyp_source_dir=None):
  """Create Android.mk for the Android framework's external/skia.

  Builds Android.mk using Skia's gyp files.

  Args:
    target_dir: Directory in which to place 'Android.mk'. If None, the file
      will be placed in skia's root directory.
    require_sk_user_config: If True, raise an AssertionError if
      SkUserConfig.h does not exist.
    gyp_source_dir: Source directory for gyp.
  """
  # Create a temporary folder to hold gyp and gypd files. Create it in SKIA_DIR
  # so that it is a sibling of gyp/, so the relationships between gyp files and
  # other files (e.g. platform_tools/android/gyp/dependencies.gypi, referenced
  # by android_deps.gyp as a relative path) is unchanged.
  # Use mkdtemp to find an unused folder name, but then delete it so copytree
  # can be called with a non-existent directory.
  tmp_folder = tempfile.mkdtemp(dir=SKIA_DIR)
  os.rmdir(tmp_folder)
  shutil.copytree(os.path.join(SKIA_DIR, GYP_FOLDER), tmp_folder)

  try:
    main_gyp_file = 'android_framework_lib.gyp'

    print 'Creating Android.mk',

    # Generate a separate VarsDict for each architecture type.  For each
    # archtype:
    # 1. call android_framework_gyp.main() to generate gypd files
    # 2. call parse_gypd to read those gypd files into the VarsDict
    # 3. delete the gypd files
    #
    # Once we have the VarsDict for each architecture type, we combine them all
    # into a single Android.mk file, which can build targets of any
    # architecture type.

    # The default uses a non-existant archtype, to find all the general
    # variable definitions.
    default_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'other',
                                         False, gyp_source_dir)
    arm_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'arm', False,
                                     gyp_source_dir)
    arm_neon_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'arm',
                                          True, gyp_source_dir)
    x86_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'x86', False,
                                     gyp_source_dir)
    x86_64_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'x86_64',
                                        False, gyp_source_dir)

    mips_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'mips', False,
                                      gyp_source_dir)

    mips64_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'mips64',
                                        False, gyp_source_dir)

    arm64_var_dict = generate_var_dict(tmp_folder, main_gyp_file, 'arm64',
                                       False, gyp_source_dir)

    # Compute the intersection of all targets. All the files in the intersection
    # should be part of the makefile always. Each dict will now contain trimmed
    # lists containing only variable definitions specific to that configuration.
    var_dict_list = [default_var_dict, arm_var_dict, arm_neon_var_dict,
                     x86_var_dict, x86_64_var_dict, mips_var_dict,
                     mips64_var_dict, arm64_var_dict]
    common = vars_dict_lib.intersect(var_dict_list)

    common.LOCAL_MODULE.add('libskia')

    # Create SkUserConfig
    user_config = os.path.join(SKIA_DIR, 'include', 'config', 'SkUserConfig.h')
    if target_dir:
      dst_dir = target_dir
    else:
      dst_dir = os.path.join(SKIA_DIR, 'include', 'core')

    generate_user_config.generate_user_config(
        original_sk_user_config=user_config,
        require_sk_user_config=require_sk_user_config, target_dir=dst_dir,
        defines=common.DEFINES)

    tool_makefile_writer.generate_tool(gyp_dir=tmp_folder,
                                       target_file='bench.gyp',
                                       skia_trunk=target_dir,
                                       dest_dir='bench',
                                       skia_lib_var_dict=common,
                                       local_module_name='skia_nanobench',
                                       local_module_tags=['tests'],
                                       desired_targets=['nanobench'],
                                       gyp_source_dir=gyp_source_dir)

    tool_makefile_writer.generate_tool(gyp_dir=tmp_folder,
                                       target_file='dm.gyp',
                                       skia_trunk=target_dir,
                                       dest_dir='dm',
                                       skia_lib_var_dict=common,
                                       local_module_name='skia_dm',
                                       local_module_tags=['tests'],
                                       desired_targets=['dm'],
                                       gyp_source_dir=gyp_source_dir)

    # Now that the defines have been written to SkUserConfig and they've been
    # used to skip adding them to the tools makefiles, they are not needed in
    # Android.mk. Reset DEFINES.
    common.DEFINES.reset()

    # Further trim arm_neon_var_dict with arm_var_dict. After this call,
    # arm_var_dict (which will now be the intersection) includes all definitions
    # used by both arm and arm + neon, and arm_neon_var_dict will only contain
    # those specific to arm + neon.
    arm_var_dict = vars_dict_lib.intersect([arm_var_dict, arm_neon_var_dict])

    # Now create a list of VarsDictData holding everything but common.
    deviations_from_common = []
    deviations_from_common.append(makefile_writer.VarsDictData(
        arm_var_dict, 'arm'))
    deviations_from_common.append(makefile_writer.VarsDictData(
        arm_neon_var_dict, 'arm', 'ARCH_ARM_HAVE_NEON'))
    deviations_from_common.append(makefile_writer.VarsDictData(x86_var_dict,
                                                               'x86'))
    deviations_from_common.append(makefile_writer.VarsDictData(x86_64_var_dict,
                                                               'x86_64'))

    deviations_from_common.append(makefile_writer.VarsDictData(mips_var_dict,
                                                               'mips'))

    deviations_from_common.append(makefile_writer.VarsDictData(mips64_var_dict,
                                                               'mips64'))

    deviations_from_common.append(makefile_writer.VarsDictData(arm64_var_dict,
                                                               'arm64'))

    makefile_writer.write_android_mk(target_dir=target_dir,
        common=common, deviations_from_common=deviations_from_common)

    makefile_writer.write_static_deps_mk(target_dir=target_dir,
        common=common, deviations_from_common=deviations_from_common)

  finally:
    shutil.rmtree(tmp_folder)

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--gyp_source_dir', help='Source of gyp program. '
                      'e.g. <path_to_skia>/third_party/externals/gyp')
  args = parser.parse_args()

  main(gyp_source_dir=args.gyp_source_dir)
