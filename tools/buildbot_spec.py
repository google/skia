#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

#!/usr/bin/env python

usage = '''
Write buildbot spec to outfile based on the bot name:
  $ python buildbot_spec.py outfile Test-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug
Or run self-tests:
  $ python buildbot_spec.py test
'''

import inspect
import json
import os
import sys

import builder_name_schema
import dm_flags
import nanobench_flags


CONFIG_COVERAGE = 'Coverage'
CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'


def lineno():
  caller = inspect.stack()[1]  # Up one level to our caller.
  return inspect.getframeinfo(caller[0]).lineno

# Since we don't actually start coverage until we're in the self-test,
# some function def lines aren't reported as covered. Add them to this
# list so that we can ignore them.
cov_skip = []

cov_start = lineno()+1   # We care about coverage starting just past this def.
def gyp_defines(builder_dict):
  gyp_defs = {}

  # skia_arch_type.
  if builder_dict['role'] == builder_name_schema.BUILDER_ROLE_BUILD:
    arch = builder_dict['target_arch']
  elif builder_dict['role'] == builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
    arch = None
  else:
    arch = builder_dict['arch']

  arch_types = {
    'x86':      'x86',
    'x86_64':   'x86_64',
    'Arm7':     'arm',
    'Arm64':    'arm64',
    'Mips':     'mips32',
    'Mips64':   'mips64',
    'MipsDSP2': 'mips32',
  }
  if arch in arch_types:
    gyp_defs['skia_arch_type']  = arch_types[arch]

  # housekeeper: build shared lib.
  if builder_dict['role'] == builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
    gyp_defs['skia_shared_lib'] = '1'

  # skia_gpu.
  if builder_dict.get('cpu_or_gpu') == 'CPU':
    gyp_defs['skia_gpu'] = '0'

  # skia_warnings_as_errors.
  werr = False
  if builder_dict['role'] == builder_name_schema.BUILDER_ROLE_BUILD:
    if 'Win' in builder_dict.get('os', ''):
      if not ('GDI' in builder_dict.get('extra_config', '') or
              'Exceptions' in builder_dict.get('extra_config', '')):
        werr = True
    elif ('Mac' in builder_dict.get('os', '') and
          'Android' in builder_dict.get('extra_config', '')):
      werr = False
    elif 'Fast' in builder_dict.get('extra_config', ''):
      # See https://bugs.chromium.org/p/skia/issues/detail?id=5257
      werr = False
    else:
      werr = True
  gyp_defs['skia_warnings_as_errors'] = str(int(werr))  # True/False -> '1'/'0'

  # Win debugger.
  if 'Win' in builder_dict.get('os', ''):
    gyp_defs['skia_win_debuggers_path'] = 'c:/DbgHelp'

  # Qt SDK (Win).
  if 'Win' in builder_dict.get('os', ''):
    if builder_dict.get('os') == 'Win8':
      gyp_defs['qt_sdk'] = 'C:/Qt/Qt5.1.0/5.1.0/msvc2012_64/'
    else:
      gyp_defs['qt_sdk'] = 'C:/Qt/4.8.5/'

  # ANGLE.
  if builder_dict.get('extra_config') == 'ANGLE':
    gyp_defs['skia_angle'] = '1'
    if builder_dict.get('os', '') in ('Ubuntu', 'Linux'):
      gyp_defs['use_x11'] = '1'
      gyp_defs['chromeos'] = '0'

  # GDI.
  if builder_dict.get('extra_config') == 'GDI':
    gyp_defs['skia_gdi'] = '1'

  # Build with Exceptions on Windows.
  if ('Win' in builder_dict.get('os', '') and
      builder_dict.get('extra_config') == 'Exceptions'):
    gyp_defs['skia_win_exceptions'] = '1'

  # iOS.
  if (builder_dict.get('os') == 'iOS' or
      builder_dict.get('extra_config') == 'iOS'):
    gyp_defs['skia_os'] = 'ios'

  # Shared library build.
  if builder_dict.get('extra_config') == 'Shared':
    gyp_defs['skia_shared_lib'] = '1'

  # Build fastest Skia possible.
  if builder_dict.get('extra_config') == 'Fast':
    gyp_defs['skia_fast'] = '1'

  # PDF viewer in GM.
  if (builder_dict.get('os') == 'Mac10.8' and
      builder_dict.get('arch') == 'x86_64' and
      builder_dict.get('configuration') == 'Release'):
    gyp_defs['skia_run_pdfviewer_in_gm'] = '1'

  # Clang.
  if builder_dict.get('compiler') == 'Clang':
    gyp_defs['skia_clang_build'] = '1'

  # Valgrind.
  if 'Valgrind' in builder_dict.get('extra_config', ''):
    gyp_defs['skia_release_optimization_level'] = '1'

  # Link-time code generation just wastes time on compile-only bots.
  if (builder_dict.get('role') == builder_name_schema.BUILDER_ROLE_BUILD and
      builder_dict.get('compiler') == 'MSVC'):
    gyp_defs['skia_win_ltcg'] = '0'

  # Mesa.
  if (builder_dict.get('extra_config') == 'Mesa' or
      builder_dict.get('cpu_or_gpu_value') == 'Mesa'):
    gyp_defs['skia_mesa'] = '1'

  # VisualBench
  if builder_dict.get('extra_config') == 'VisualBench':
    gyp_defs['skia_use_sdl'] = '1'

  # skia_use_android_framework_defines.
  if builder_dict.get('extra_config') == 'Android_FrameworkDefs':
    gyp_defs['skia_use_android_framework_defines'] = '1'

  # Skia dump stats for perf tests and gpu
  if (builder_dict.get('cpu_or_gpu') == 'GPU' and
      builder_dict.get('role') == 'Perf'):
      gyp_defs['skia_dump_stats'] = '1'

  # CommandBuffer.
  if builder_dict.get('extra_config') == 'CommandBuffer':
    gyp_defs['skia_command_buffer'] = '1'

  # Vulkan.
  if builder_dict.get('extra_config') == 'Vulkan':
    gyp_defs['skia_vulkan'] = '1'

  return gyp_defs


cov_skip.extend([lineno(), lineno() + 1])
def get_extra_env_vars(builder_dict):
  env = {}
  if builder_dict.get('configuration') == CONFIG_COVERAGE:
    # We have to use Clang 3.6 because earlier versions do not support the
    # compile flags we use and 3.7 and 3.8 hit asserts during compilation.
    env['CC'] = '/usr/bin/clang-3.6'
    env['CXX'] = '/usr/bin/clang++-3.6'
  elif builder_dict.get('compiler') == 'Clang':
    env['CC'] = '/usr/bin/clang'
    env['CXX'] = '/usr/bin/clang++'

  # SKNX_NO_SIMD, SK_USE_DISCARDABLE_SCALEDIMAGECACHE, etc.
  extra_config = builder_dict.get('extra_config', '')
  if extra_config.startswith('SK') and extra_config.isupper():
    env['CPPFLAGS'] = '-D' + extra_config

  return env


cov_skip.extend([lineno(), lineno() + 1])
def build_targets_from_builder_dict(builder_dict, do_test_steps, do_perf_steps):
  """Return a list of targets to build, depending on the builder type."""
  if builder_dict['role'] in ('Test', 'Perf') and builder_dict['os'] == 'iOS':
    return ['iOSShell']
  if builder_dict.get('extra_config') == 'Appurify':
    return ['VisualBenchTest_APK']
  if 'SAN' in builder_dict.get('extra_config', ''):
    # 'most' does not compile under MSAN.
    return ['dm', 'nanobench']
  t = []
  if do_test_steps:
    t.append('dm')
  if builder_dict.get('extra_config') == 'VisualBench':
    t.append('visualbench')
  elif do_perf_steps:
    t.append('nanobench')
  if t:
    return t
  else:
    return ['most']


cov_skip.extend([lineno(), lineno() + 1])
def device_cfg(builder_dict):
  # Android.
  if 'Android' in builder_dict.get('extra_config', ''):
    if 'NoNeon' in builder_dict['extra_config']:
      return 'arm_v7'
    return {
      'Arm64': 'arm64',
      'x86': 'x86',
      'x86_64': 'x86_64',
      'Mips': 'mips',
      'Mips64': 'mips64',
      'MipsDSP2': 'mips_dsp2',
    }.get(builder_dict['target_arch'], 'arm_v7_neon')
  elif builder_dict.get('os') == 'Android':
    return {
      'AndroidOne':    'arm_v7_neon',
      'GalaxyS3':      'arm_v7_neon',
      'GalaxyS4':      'arm_v7_neon',
      'NVIDIA_Shield': 'arm64',
      'Nexus10':       'arm_v7_neon',
      'Nexus5':        'arm_v7_neon',
      'Nexus6':        'arm_v7_neon',
      'Nexus7':        'arm_v7_neon',
      'Nexus7v2':      'arm_v7_neon',
      'Nexus9':        'arm64',
      'NexusPlayer':   'x86',
    }[builder_dict['model']]

  # iOS.
  if 'iOS' in builder_dict.get('os', ''):
    return {
      'iPad4': 'iPad4,1',
    }[builder_dict['model']]

  return None


cov_skip.extend([lineno(), lineno() + 1])
def product_board(builder_dict):
  if 'Android' in builder_dict.get('os', ''):
    return {
      'AndroidOne':    'sprout',
      'GalaxyS3':      'm0',  #'smdk4x12', Detected incorrectly by swarming?
      'GalaxyS4':      None,  # TODO(borenet,kjlubick)
      'NVIDIA_Shield': 'foster',
      'Nexus10':       'manta',
      'Nexus5':        'hammerhead',
      'Nexus6':        'shamu',
      'Nexus7':        'grouper',
      'Nexus7v2':      'flo',
      'Nexus9':        'flounder',
      'NexusPlayer':   'fugu',
    }[builder_dict['model']]
  return None


cov_skip.extend([lineno(), lineno() + 1])
def get_builder_spec(builder_name):
  builder_dict = builder_name_schema.DictForBuilderName(builder_name)
  env = get_extra_env_vars(builder_dict)
  gyp_defs = gyp_defines(builder_dict)
  gyp_defs_list = ['%s=%s' % (k, v) for k, v in gyp_defs.iteritems()]
  gyp_defs_list.sort()
  env['GYP_DEFINES'] = ' '.join(gyp_defs_list)
  rv = {
    'builder_cfg': builder_dict,
    'dm_flags': dm_flags.get_args(builder_name),
    'env': env,
    'nanobench_flags': nanobench_flags.get_args(builder_name),
  }
  device = device_cfg(builder_dict)
  if device:
    rv['device_cfg'] = device
  board = product_board(builder_dict)
  if board:
    rv['product.board'] = board

  role = builder_dict['role']
  if role == builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
    configuration = CONFIG_RELEASE
  else:
    configuration = builder_dict.get(
        'configuration', CONFIG_DEBUG)
  arch = (builder_dict.get('arch') or builder_dict.get('target_arch'))
  if ('Win' in builder_dict.get('os', '') and arch == 'x86_64'):
    configuration += '_x64'
  rv['configuration'] = configuration
  if configuration == CONFIG_COVERAGE:
    rv['do_compile_steps'] = False
  rv['do_test_steps'] = role == builder_name_schema.BUILDER_ROLE_TEST
  rv['do_perf_steps'] = (role == builder_name_schema.BUILDER_ROLE_PERF or
                         (role == builder_name_schema.BUILDER_ROLE_TEST and
                          configuration == CONFIG_DEBUG))
  if rv['do_test_steps'] and 'Valgrind' in builder_name:
    rv['do_perf_steps'] = True
  if 'GalaxyS4' in builder_name:
    rv['do_perf_steps'] = False

  rv['build_targets'] = build_targets_from_builder_dict(
        builder_dict, rv['do_test_steps'], rv['do_perf_steps'])

  # Do we upload perf results?
  upload_perf_results = False
  if role == builder_name_schema.BUILDER_ROLE_PERF:
    upload_perf_results = True
  rv['upload_perf_results'] = upload_perf_results

  # Do we upload correctness results?
  skip_upload_bots = [
    'ASAN',
    'Coverage',
    'MSAN',
    'TSAN',
    'UBSAN',
    'Valgrind',
  ]
  upload_dm_results = True
  for s in skip_upload_bots:
    if s in builder_name:
      upload_dm_results = False
      break
  rv['upload_dm_results'] = upload_dm_results

  return rv


cov_end = lineno()   # Don't care about code coverage past here.


def self_test():
  import coverage  # This way the bots don't need coverage.py to be installed.
  args = {}
  cases = [
        'Build-Mac10.8-Clang-Arm7-Debug-Android',
        'Build-Win-MSVC-x86-Debug',
        'Build-Win-MSVC-x86-Debug-GDI',
        'Build-Win-MSVC-x86-Debug-Exceptions',
        'Build-Ubuntu-GCC-Arm7-Debug-Android_FrameworkDefs',
        'Build-Ubuntu-GCC-Arm7-Debug-Android_NoNeon',
        'Build-Ubuntu-GCC-x86_64-Release-ANGLE',
        'Build-Ubuntu-GCC-x64_64-Release-Fast',
        'Build-Ubuntu-GCC-x86_64-Release-Mesa',
        'Housekeeper-PerCommit',
        'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot',
        'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX660-x86_64-Release-VisualBench',
        'Test-Android-GCC-GalaxyS4-GPU-SGX544-Arm7-Debug',
        'Perf-Android-GCC-Nexus5-GPU-Adreno330-Arm7-Release-Appurify',
        'Test-Android-GCC-Nexus6-GPU-Adreno420-Arm7-Debug',
        'Test-iOS-Clang-iPad4-GPU-SGX554-Arm7-Debug',
        'Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-CommandBuffer',
        'Test-Mac10.8-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Release',
        'Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Coverage',
        'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-MSAN',
        ('Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-'
         'SK_USE_DISCARDABLE_SCALEDIMAGECACHE'),
        'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-SKNX_NO_SIMD',
        'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Fast',
        'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Shared',
        'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
        'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-Vulkan',
        'Test-Win8-MSVC-ShuttleB-GPU-HD4600-x86-Release-ANGLE',
        'Test-Win8-MSVC-ShuttleA-CPU-AVX-x86_64-Debug',
  ]

  cov = coverage.coverage()
  cov.start()
  for case in cases:
    args[case] = get_builder_spec(case)
  cov.stop()

  this_file = os.path.basename(__file__)
  _, _, not_run, _ = cov.analysis(this_file)
  filtered = [line for line in not_run if
              line > cov_start and line < cov_end and line not in cov_skip]
  if filtered:
    print 'Lines not covered by test cases: ', filtered
    sys.exit(1)

  golden = this_file.replace('.py', '.json')
  with open(os.path.join(os.path.dirname(__file__), golden), 'w') as f:
    json.dump(args, f, indent=2, sort_keys=True)


if __name__ == '__main__':
  if len(sys.argv) == 2 and sys.argv[1] == 'test':
    self_test()
    sys.exit(0)

  if len(sys.argv) != 3:
    print usage
    sys.exit(1)

  with open(sys.argv[1], 'w') as out:
    json.dump(get_builder_spec(sys.argv[2]), out)
