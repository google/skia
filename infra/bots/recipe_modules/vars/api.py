# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api
import os


BOTO_CHROMIUM_SKIA_GM = 'chromium-skia-gm.boto'

CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'


def get_gyp_defines(builder_dict):
  gyp_defs = {}

  # skia_arch_type.
  if builder_dict['role'] == 'Build':
    arch = builder_dict['target_arch']
  elif builder_dict['role'] in ('Housekeeper', 'Infra'):
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
  if builder_dict['role'] == 'Housekeeper':
    gyp_defs['skia_shared_lib'] = '1'

  # skia_gpu.
  if builder_dict.get('cpu_or_gpu') == 'CPU':
    gyp_defs['skia_gpu'] = '0'

  # skia_warnings_as_errors.
  werr = False
  if builder_dict['role'] == 'Build':
    if 'Win' in builder_dict.get('os', ''):
      if not ('GDI' in builder_dict.get('extra_config', '') or
              'Exceptions' in builder_dict.get('extra_config', '')):
        werr = True
    elif ('Mac' in builder_dict.get('os', '') and
          'Android' in builder_dict.get('extra_config', '')):
      werr = False
    elif 'Fast' in builder_dict.get('extra_config', ''):  # pragma: no cover
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
  if builder_dict.get('extra_config') == 'ANGLE':  # pragma: no cover
    gyp_defs['skia_angle'] = '1'
    if builder_dict.get('os', '') in ('Ubuntu', 'Linux'):
      gyp_defs['use_x11'] = '1'
      gyp_defs['chromeos'] = '0'

  # GDI.
  if builder_dict.get('extra_config') == 'GDI':  # pragma: no cover
    gyp_defs['skia_gdi'] = '1'

  # Build with Exceptions on Windows.
  if ('Win' in builder_dict.get('os', '') and
      builder_dict.get('extra_config') == 'Exceptions'):  # pragma: no cover
    gyp_defs['skia_win_exceptions'] = '1'

  # iOS.
  if (builder_dict.get('os') == 'iOS' or
      builder_dict.get('extra_config') == 'iOS'):
    gyp_defs['skia_os'] = 'ios'

  # Shared library build.
  if builder_dict.get('extra_config') == 'Shared':
    gyp_defs['skia_shared_lib'] = '1'

  # Build fastest Skia possible.
  if builder_dict.get('extra_config') == 'Fast':  # pragma: no cover
    gyp_defs['skia_fast'] = '1'

  # PDF viewer in GM.
  if (builder_dict.get('os') == 'Mac10.8' and
      builder_dict.get('arch') == 'x86_64' and
      builder_dict.get('configuration') == 'Release'):  # pragma: no cover
    gyp_defs['skia_run_pdfviewer_in_gm'] = '1'

  # Clang.
  if builder_dict.get('compiler') == 'Clang':
    gyp_defs['skia_clang_build'] = '1'

  # Valgrind.
  if 'Valgrind' in builder_dict.get('extra_config', ''):
    gyp_defs['skia_release_optimization_level'] = '1'

  # Link-time code generation just wastes time on compile-only bots.
  if (builder_dict.get('role') == 'Build' and
      builder_dict.get('compiler') == 'MSVC'):
    gyp_defs['skia_win_ltcg'] = '0'

  # Mesa.
  if (builder_dict.get('extra_config') == 'Mesa' or
      builder_dict.get('cpu_or_gpu_value') == 'Mesa'):  # pragma: no cover
    gyp_defs['skia_mesa'] = '1'

  # skia_use_android_framework_defines.
  if builder_dict.get('extra_config') == 'Android_FrameworkDefs':
    gyp_defs['skia_use_android_framework_defines'] = '1'  # pragma: no cover

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
    gyp_defs['skia_vulkan_debug_layers'] = '0'

  return gyp_defs


def get_extra_env_vars(builder_dict):
  env = {}
  if builder_dict.get('compiler') == 'Clang':
    env['CC'] = '/usr/bin/clang'
    env['CXX'] = '/usr/bin/clang++'

  # SKNX_NO_SIMD, SK_USE_DISCARDABLE_SCALEDIMAGECACHE, etc.
  extra_config = builder_dict.get('extra_config', '')
  if extra_config.startswith('SK') and extra_config.isupper():
    env['CPPFLAGS'] = '-D' + extra_config  # pragma: no cover

  return env


def build_targets_from_builder_dict(builder_dict):
  """Return a list of targets to build, depending on the builder type."""
  if builder_dict.get('extra_config') == 'iOS':
    return ['iOSShell']
  if 'SAN' in builder_dict.get('extra_config', ''):
    # 'most' does not compile under MSAN.
    return ['dm', 'nanobench']
  else:
    return ['most']


def device_cfg(builder_dict):
  # Android.
  if 'Android' in builder_dict.get('extra_config', ''):
    if 'NoNeon' in builder_dict['extra_config']:  # pragma: no cover
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


def dm_flags(bot):
  args = []

  # 32-bit desktop bots tend to run out of memory, because they have relatively
  # far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
  if '-x86-' in bot and not 'NexusPlayer' in bot:
    args.extend('--threads 4'.split(' '))

  # These are the canonical configs that we would ideally run on all bots. We
  # may opt out or substitute some below for specific bots
  configs = ['565', '8888', 'gpu', 'gpusrgb', 'pdf']
  # Add in either msaa4 or msaa16 to the canonical set of configs to run
  if 'Android' in bot or 'iOS' in bot:
    configs.append('msaa4')
  else:
    configs.append('msaa16')

  # With msaa, the S4 crashes and the NP produces a long error stream when we
  # run with MSAA. The Tegra2 and Tegra3 just don't support it. No record of
  # why we're not running msaa on iOS, probably started with gpu config and just
  # haven't tried.
  if ('GalaxyS4'    in bot or
      'NexusPlayer' in bot or
      'Tegra3'      in bot or
      'iOS'         in bot):
    configs = [x for x in configs if 'msaa' not in x]

  # Runs out of memory on Android bots and Daisy.  Everyone else seems fine.
  if 'Android' in bot or 'Daisy' in bot:
    configs.remove('pdf')

  if '-GCE-' in bot:
    configs.extend(['f16', 'srgb'])              # Gamma-correct formats.
    configs.extend(['sp-8888', '2ndpic-8888'])   # Test niche uses of SkPicture.

  if '-TSAN' not in bot:
    if ('TegraK1'  in bot or
        'TegraX1'  in bot or
        'GTX550Ti' in bot or
        'GTX660'   in bot or
        'GT610'    in bot):
      if 'Android' in bot:
        configs.append('nvprdit4')
      else:
        configs.append('nvprdit16')

  # We want to test the OpenGL config not the GLES config on the X1
  if 'TegraX1' in bot:
    configs = [x.replace('gpu', 'gl') for x in configs]
    configs = [x.replace('msaa', 'glmsaa') for x in configs]
    configs = [x.replace('nvpr', 'glnvpr') for x in configs]

  # NP is running out of RAM when we run all these modes.  skia:3255
  if 'NexusPlayer' not in bot:
    configs.extend(mode + '-8888' for mode in
                   ['serialize', 'tiles_rt', 'pic'])

  if 'ANGLE' in bot:  # pragma: no cover
    configs.append('angle')

  # We want to run gpudft on atleast the mali 400
  if 'GalaxyS3' in bot:
    configs.append('gpudft')

  # Test instanced rendering on a limited number of platforms
  if 'Nexus6' in bot:  # pragma: no cover
    configs.append('esinst') # esinst4 isn't working yet on Adreno.
  elif 'TegraX1' in bot:
    # Multisampled instanced configs use nvpr.
    configs = [x.replace('glnvpr', 'glinst') for x in configs]
    configs.append('glinst')
  elif 'MacMini6.2' in bot:
    configs.extend(['glinst', 'glinst16'])

  # CommandBuffer bot *only* runs the command_buffer config.
  if 'CommandBuffer' in bot:
    configs = ['commandbuffer']

  # Vulkan bot *only* runs the vk config.
  if 'Vulkan' in bot:
    configs = ['vk']

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image colorImage'.split(' '))

  if 'GalaxyS' in bot:
    args.extend(('--threads', '0'))

  blacklist = []

  # TODO: ???
  blacklist.extend('f16 _ _ dstreadshuffle'.split(' '))
  blacklist.extend('f16 image _ _'.split(' '))
  blacklist.extend('srgb image _ _'.split(' '))
  blacklist.extend('gpusrgb image _ _'.split(' '))

  if 'Valgrind' in bot:
    # These take 18+ hours to run.
    blacklist.extend('pdf gm _ fontmgr_iter'.split(' '))
    blacklist.extend('pdf _ _ PANO_20121023_214540.jpg'.split(' '))
    blacklist.extend('pdf skp _ worldjournal'.split(' '))
    blacklist.extend('pdf skp _ desk_baidu.skp'.split(' '))
    blacklist.extend('pdf skp _ desk_wikipedia.skp'.split(' '))

  if 'iOS' in bot:
    blacklist.extend('gpu skp _ _ msaa skp _ _'.split(' '))
    blacklist.extend('msaa16 gm _ tilemodesProcess'.split(' '))

  if 'Mac' in bot or 'iOS' in bot:
    # CG fails on questionable bmps
    blacklist.extend('_ image gen_platf rgba32abf.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rgb24prof.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rgb24lprof.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 8bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 4bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 32bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 24bpp-pixeldata-cropped.bmp'.split(' '))

    # CG has unpredictable behavior on this questionable gif
    # It's probably using uninitialized memory
    blacklist.extend('_ image gen_platf frame_larger_than_image.gif'.split(' '))

  # WIC fails on questionable bmps
  if 'Win' in bot:
    blacklist.extend('_ image gen_platf rle8-height-negative.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rle4-height-negative.bmp'.split(' '))
    blacklist.extend('_ image gen_platf pal8os2v2.bmp'.split(' '))
    blacklist.extend('_ image gen_platf pal8os2v2-16.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rgba32abf.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rgb24prof.bmp'.split(' '))
    blacklist.extend('_ image gen_platf rgb24lprof.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 8bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 4bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 32bpp-pixeldata-cropped.bmp'.split(' '))
    blacklist.extend('_ image gen_platf 24bpp-pixeldata-cropped.bmp'.split(' '))
    if 'x86_64' in bot and 'CPU' in bot:
      # This GM triggers a SkSmallAllocator assert.
      blacklist.extend('_ gm _ composeshader_bitmap'.split(' '))

  if 'Android' in bot or 'iOS' in bot:
    # This test crashes the N9 (perhaps because of large malloc/frees). It also
    # is fairly slow and not platform-specific. So we just disable it on all of
    # Android and iOS. skia:5438
    blacklist.extend('_ test _ GrShape'.split(' '))

  if 'Win8' in bot:
    # bungeman: "Doesn't work on Windows anyway, produces unstable GMs with
    # 'Unexpected error' from DirectWrite"
    blacklist.extend('_ gm _ fontscalerdistortable'.split(' '))

  # skia:4095
  bad_serialize_gms = ['bleed_image',
                       'c_gms',
                       'colortype',
                       'colortype_xfermodes',
                       'drawfilter',
                       'fontmgr_bounds_0.75_0',
                       'fontmgr_bounds_1_-0.25',
                       'fontmgr_bounds',
                       'fontmgr_match',
                       'fontmgr_iter']

  # skia:5589
  bad_serialize_gms.extend(['bitmapfilters',
                            'bitmapshaders',
                            'bleed',
                            'bleed_alpha_bmp',
                            'bleed_alpha_bmp_shader',
                            'convex_poly_clip',
                            'extractalpha',
                            'filterbitmap_checkerboard_32_32_g8',
                            'filterbitmap_image_mandrill_64',
                            'shadows',
                            'simpleaaclip_aaclip'])
  # skia:5595
  bad_serialize_gms.extend(['composeshader_bitmap',
                            'scaled_tilemodes_npot',
                            'scaled_tilemodes'])
  for test in bad_serialize_gms:
    blacklist.extend(['serialize-8888', 'gm', '_', test])

  if 'Mac' not in bot:
    for test in ['bleed_alpha_image', 'bleed_alpha_image_shader']:
      blacklist.extend(['serialize-8888', 'gm', '_', test])
  # It looks like we skip these only for out-of-memory concerns.
  if 'Win' in bot or 'Android' in bot:
    for test in ['verylargebitmap', 'verylarge_picture_image']:
      blacklist.extend(['serialize-8888', 'gm', '_', test])

  # skia:4769
  for test in ['drawfilter']:
    blacklist.extend([    'sp-8888', 'gm', '_', test])
    blacklist.extend([   'pic-8888', 'gm', '_', test])
    blacklist.extend(['2ndpic-8888', 'gm', '_', test])
  # skia:4703
  for test in ['image-cacherator-from-picture',
               'image-cacherator-from-raster',
               'image-cacherator-from-ctable']:
    blacklist.extend([       'sp-8888', 'gm', '_', test])
    blacklist.extend([      'pic-8888', 'gm', '_', test])
    blacklist.extend([   '2ndpic-8888', 'gm', '_', test])
    blacklist.extend(['serialize-8888', 'gm', '_', test])

  # Extensions for RAW images
  r = ["arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
       "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW"]

  # skbug.com/4888
  # Blacklist RAW images (and a few large PNGs) on GPU bots
  # until we can resolve failures
  if 'GPU' in bot:
    blacklist.extend('_ image _ interlaced1.png'.split(' '))
    blacklist.extend('_ image _ interlaced2.png'.split(' '))
    blacklist.extend('_ image _ interlaced3.png'.split(' '))
    for raw_ext in r:
      blacklist.extend(('_ image _ .%s' % raw_ext).split(' '))

  if 'Nexus9' in bot:  # pragma: no cover
    for raw_ext in r:
      blacklist.extend(('_ image _ .%s' % raw_ext).split(' '))

  # Large image that overwhelms older Mac bots
  if 'MacMini4.1-GPU' in bot:  # pragma: no cover
    blacklist.extend('_ image _ abnormal.wbmp'.split(' '))
    blacklist.extend(['msaa16', 'gm', '_', 'blurcircles'])

  match = []
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')

  if 'GalaxyS3' in bot:  # skia:1699
    match.append('~WritePixels')

  if 'AndroidOne' in bot:  # skia:4711
    match.append('~WritePixels')  # pragma: no cover

  if 'NexusPlayer' in bot:  # pragma: no cover
    match.append('~ResourceCache')

  if 'Nexus10' in bot: # skia:5509
    match.append('~CopySurface')  # pragma: no cover

  if 'ANGLE' in bot and 'Debug' in bot:  # pragma: no cover
    match.append('~GLPrograms') # skia:4717

  if 'MSAN' in bot:
    match.extend(['~Once', '~Shared'])  # Not sure what's up with these tests.

  if 'TSAN' in bot:  # pragma: no cover
    match.extend(['~ReadWriteAlpha'])   # Flaky on TSAN-covered on nvidia bots.

  if blacklist:
    args.append('--blacklist')
    args.extend(blacklist)

  if match:
    args.append('--match')
    args.extend(match)

  # These bots run out of memory running RAW codec tests. Do not run them in
  # parallel
  if ('NexusPlayer' in bot or 'Nexus5' in bot or 'Nexus9' in bot
      or 'Win8-MSVC-ShuttleB' in bot):
    args.append('--noRAW_threading')

  return args


def get_builder_spec(api, builder_name):
  builder_dict = api.builder_name_schema.DictForBuilderName(builder_name)
  env = get_extra_env_vars(builder_dict)
  gyp_defs = get_gyp_defines(builder_dict)
  gyp_defs_list = ['%s=%s' % (k, v) for k, v in gyp_defs.iteritems()]
  gyp_defs_list.sort()
  env['GYP_DEFINES'] = ' '.join(gyp_defs_list)

  build_targets = build_targets_from_builder_dict(builder_dict)
  rv = {
    'build_targets': build_targets,
    'builder_cfg': builder_dict,
    'dm_flags': dm_flags(builder_name),
    'env': env,
    'nanobench_flags': nanobench_flags(builder_name),
  }
  device = device_cfg(builder_dict)
  if device:
    rv['device_cfg'] = device
  board = product_board(builder_dict)
  if board:
    rv['product.board'] = board

  role = builder_dict['role']
  if role == api.builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
    configuration = CONFIG_RELEASE
  else:
    configuration = builder_dict.get('configuration', CONFIG_DEBUG)
  arch = (builder_dict.get('arch') or builder_dict.get('target_arch'))
  if ('Win' in builder_dict.get('os', '') and arch == 'x86_64'):
    configuration += '_x64'
  rv['configuration'] = configuration
  if configuration == 'Coverage':
    rv['do_compile_steps'] = False
  rv['do_test_steps'] = role == api.builder_name_schema.BUILDER_ROLE_TEST
  rv['do_perf_steps'] = role == api.builder_name_schema.BUILDER_ROLE_PERF

  # Do we upload perf results?
  upload_perf_results = False
  if (role == api.builder_name_schema.BUILDER_ROLE_PERF and
      CONFIG_RELEASE in configuration):
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


def nanobench_flags(bot):
  args = ['--pre_log']

  if 'GPU' in bot:
    args.append('--images')
    args.extend(['--gpuStatsDump', 'true'])

  if 'Android' in bot and 'GPU' in bot:
    args.extend(['--useThermalManager', '1,1,10,1000'])

  args.extend(['--scales', '1.0', '1.1'])

  if 'iOS' in bot:
    args.extend(['--skps', 'ignore_skps'])

  config = ['565', '8888', 'gpu', 'nonrendering', 'angle', 'hwui' ]
  config += [ 'f16', 'srgb' ]
  # The S4 crashes and the NP produces a long error stream when we run with
  # MSAA.
  if ('GalaxyS4'    not in bot and
      'NexusPlayer' not in bot):
    if 'Android' in bot:
      # The TegraX1 has a regular OpenGL implementation. We bench that instead
      # of ES.
      if 'TegraX1' in bot:
        config.remove('gpu')
        config.extend(['gl', 'glmsaa4', 'glnvpr4', 'glnvprdit4'])
      else:
        config.extend(['msaa4', 'nvpr4', 'nvprdit4'])
    else:
      config.extend(['msaa16', 'nvpr16', 'nvprdit16'])

  # Bench instanced rendering on a limited number of platforms
  if 'Nexus6' in bot:  # pragma: no cover
    config.append('esinst') # esinst4 isn't working yet on Adreno.
  elif 'TegraX1' in bot:
    config.extend(['glinst', 'glinst4'])
  elif 'MacMini6.2' in bot:
    config.extend(['glinst', 'glinst16'])

  if 'Vulkan' in bot:
    config = ['vk']

  args.append('--config')
  args.extend(config)

  if 'Valgrind' in bot:
    # Don't care about Valgrind performance.
    args.extend(['--loops',   '1'])
    args.extend(['--samples', '1'])
    # Ensure that the bot framework does not think we have timed out.
    args.extend(['--keepAlive', 'true'])

  match = []
  if 'Android' in bot:
    # Segfaults when run as GPU bench. Very large texture?
    match.append('~blurroundrect')
    match.append('~patch_grid')  # skia:2847
    match.append('~desk_carsvg')
  if 'NexusPlayer' in bot:  # pragma: no cover
    match.append('~desk_unicodetable')
  if 'Nexus5' in bot:  # pragma: no cover
    match.append('~keymobi_shop_mobileweb_ebay_com.skp')  # skia:5178
  if 'iOS' in bot:
    match.append('~blurroundrect')
    match.append('~patch_grid')  # skia:2847
    match.append('~desk_carsvg')
    match.append('~keymobi')
    match.append('~path_hairline')
    match.append('~GLInstancedArraysBench') # skia:4714

  # the 32-bit GCE bots run out of memory in DM when running these large images
  # so defensively disable them in nanobench, too.
  # FIXME (scroggo): This may have just been due to SkImageDecoder's
  # buildTileIndex leaking memory (https://bug.skia.org/4360). That is
  # disabled by default for nanobench, so we may not need this.
  # FIXME (scroggo): Share image blacklists between dm and nanobench?
  if 'x86' in bot and not 'x86-64' in bot:
    match.append('~interlaced1.png')
    match.append('~interlaced2.png')
    match.append('~interlaced3.png')

  # This low-end Android bot crashes about 25% of the time while running the
  # (somewhat intense) shapes benchmarks.
  if 'Perf-Android-GCC-GalaxyS3-GPU-Mali400-Arm7-Release' in bot:
    match.append('~shapes_')  # pragma: no cover

  # We do not need or want to benchmark the decodes of incomplete images.
  # In fact, in nanobench we assert that the full image decode succeeds.
  match.append('~inc0.gif')
  match.append('~inc1.gif')
  match.append('~incInterlaced.gif')
  match.append('~inc0.jpg')
  match.append('~incGray.jpg')
  match.append('~inc0.wbmp')
  match.append('~inc1.wbmp')
  match.append('~inc0.webp')
  match.append('~inc1.webp')
  match.append('~inc0.ico')
  match.append('~inc1.ico')
  match.append('~inc0.png')
  match.append('~inc1.png')
  match.append('~inc2.png')
  match.append('~inc12.png')
  match.append('~inc13.png')
  match.append('~inc14.png')
  match.append('~inc0.webp')
  match.append('~inc1.webp')

  if match:
    args.append('--match')
    args.extend(match)

  return args


class SkiaVarsApi(recipe_api.RecipeApi):

  def make_path(self, *path):
    """Return a Path object for the given path."""
    key  = 'custom_%s' % '_'.join(path)
    self.m.path.c.base_paths[key] = tuple(path)
    return self.m.path[key]

  def gsutil_env(self, boto_file):
    """Environment variables for gsutil."""
    boto_path = None
    if boto_file:
      boto_path = self.m.path.join(self.home_dir, boto_file)
    return {'AWS_CREDENTIAL_FILE': boto_path,
            'BOTO_CONFIG': boto_path}

  @property
  def home_dir(self):
    """Find the home directory."""
    home_dir = os.path.expanduser('~')
    if self._test_data.enabled:
      home_dir = '[HOME]'
    return home_dir

  def get_builder_spec(self, builder_name):
    """Return the builder_spec for the given builder name."""
    return get_builder_spec(self.m, builder_name)

  def setup(self):
    """Prepare the variables."""
    # Setup
    self.builder_name = self.m.properties['buildername']
    self.master_name = self.m.properties['mastername']
    self.slave_name = self.m.properties['slavename']
    self.build_number = self.m.properties['buildnumber']

    self.slave_dir = self.m.path['slave_build']
    self.checkout_root = self.slave_dir
    self.default_env = {}
    self.gclient_env = {}
    self.is_compile_bot = self.builder_name.startswith('Build-')

    self.default_env['CHROME_HEADLESS'] = '1'
    # The 'depot_tools' directory comes from recipe DEPS and isn't provided by
    # default. We have to set it manually.
    self.m.path.c.base_paths['depot_tools'] = (
        self.m.path.c.base_paths['slave_build'] +
        ('skia', 'infra', 'bots', '.recipe_deps', 'depot_tools'))
    if 'Win' in self.builder_name:
      self.m.path.c.base_paths['depot_tools'] = (
          'c:\\', 'Users', 'chrome-bot', 'depot_tools')

    # Compile bots keep a persistent checkout.
    self.persistent_checkout = (self.is_compile_bot or
                                'RecreateSKPs' in self.builder_name)
    if self.persistent_checkout:
      if 'Win' in self.builder_name:
        self.checkout_root = self.make_path('C:\\', 'b', 'work')
        self.gclient_cache = self.make_path('C:\\', 'b', 'cache')
      else:
        self.checkout_root = self.make_path('/', 'b', 'work')
        self.gclient_cache = self.make_path('/', 'b', 'cache')

      # got_revision is filled in after checkout steps.
      self.got_revision = None
    else:
      # If there's no persistent checkout, then we have to asume we got the
      # correct revision of the files from isolate.
      self.got_revision = self.m.properties['revision']

    self.skia_dir = self.checkout_root.join('skia')
    if not self.persistent_checkout:
      self.m.path['checkout'] = self.skia_dir

    self.infrabots_dir = self.skia_dir.join('infra', 'bots')
    self.resource_dir = self.skia_dir.join('resources')
    self.images_dir = self.slave_dir.join('skimage')
    self.skia_out = self.skia_dir.join('out', self.builder_name)
    self.swarming_out_dir = self.make_path(self.m.properties['swarm_out_dir'])
    self.local_skp_dir = self.slave_dir.join('skp')
    if not self.is_compile_bot:
      self.skia_out = self.slave_dir.join('out')
    self.tmp_dir = self.m.path['slave_build'].join('tmp')

    # Some bots also require a checkout of chromium.
    self.need_chromium_checkout = 'CommandBuffer' in self.builder_name
    if 'CommandBuffer' in self.builder_name:
      self.gclient_env['GYP_CHROMIUM_NO_ACTION'] = '0'
    if ((self.is_compile_bot and
         'SAN' in self.builder_name) or
        'RecreateSKPs' in self.builder_name):
      self.need_chromium_checkout = True
      if 'RecreateSKPs' in self.builder_name:
        self.gclient_env['CPPFLAGS'] = (
            '-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1')

    # Some bots also require a checkout of PDFium.
    self.need_pdfium_checkout = 'PDFium' in self.builder_name

    # Obtain the spec for this builder. Use it to set more properties.
    self.builder_spec = get_builder_spec(self.m, self.builder_name)

    self.builder_cfg = self.builder_spec['builder_cfg']
    self.role = self.builder_cfg['role']

    self.configuration = self.builder_spec['configuration']
    self.default_env.update({'SKIA_OUT': self.skia_out,
                             'BUILDTYPE': self.configuration})
    self.do_compile_steps = self.builder_spec.get('do_compile_steps', True)
    self.do_test_steps = self.builder_spec['do_test_steps']
    self.do_perf_steps = self.builder_spec['do_perf_steps']
    self.is_trybot = self.builder_cfg['is_trybot']
    self.issue = None
    self.patchset = None
    self.rietveld = None
    if self.is_trybot:
      self.issue = self.m.properties['issue']
      self.patchset = self.m.properties['patchset']
      self.rietveld = self.m.properties['rietveld']
    self.upload_dm_results = self.builder_spec['upload_dm_results']
    self.upload_perf_results = self.builder_spec['upload_perf_results']
    self.dm_dir = self.m.path.join(
        self.swarming_out_dir, 'dm')
    self.perf_data_dir = self.m.path.join(self.swarming_out_dir,
        'perfdata', self.builder_name, 'data')
