#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

#!/usr/bin/env python

usage = '''
Write extra flags to outfile for DM based on the bot name:
  $ python dm_flags.py outfile Test-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug
Or run self-tests:
  $ python dm_flags.py test
'''

import inspect
import json
import os
import sys


def lineno():
  caller = inspect.stack()[1]  # Up one level to our caller.
  return inspect.getframeinfo(caller[0]).lineno


cov_start = lineno()+1   # We care about coverage starting just past this def.
def get_args(bot):
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

  # NP is running out of RAM when we run all these modes.  skia:3255
  if 'NexusPlayer' not in bot:
    configs.extend(mode + '-8888' for mode in
                   ['serialize', 'tiles_rt', 'pic'])

  if 'ANGLE' in bot:
    configs.append('angle')

  # We want to run gpudft on atleast the mali 400
  if 'GalaxyS3' in bot:
      configs.append('gpudft')

  # CommandBuffer bot *only* runs the command_buffer config.
  if 'CommandBuffer' in bot:
    configs = ['commandbuffer']

  # Vulkan bot *only* runs the vk config.
  if 'Vulkan' in bot:
    configs = ['vk']

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image'.split(' '))

  if 'GalaxyS' in bot:
    args.extend(('--threads', '0'))

  blacklist = []

  # TODO: ???
  blacklist.extend('f16 _ _ dstreadshuffle'.split(' '))
  blacklist.extend('f16 image _ _'.split(' '))
  blacklist.extend('srgb image _ _'.split(' '))
  blacklist.extend('gpusrgb image _ _'.split(' '))

  # Certain gm's on win7 gpu and pdf are never finishing and keeping the test
  # running forever
  if 'Win7' in bot:
    blacklist.extend('msaa16 gm _ colorwheelnative'.split(' '))
    blacklist.extend('pdf gm _ fontmgr_iter_factory'.split(' '))

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
 
  # skia:4095
  for test in ['bleed_image',
               'c_gms',
               'colortype',
               'colortype_xfermodes',
               'drawfilter',
               'fontmgr_bounds_0.75_0',
               'fontmgr_bounds_1_-0.25',
               'fontmgr_bounds',
               'fontmgr_match',
               'fontmgr_iter']:
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

  # Large image that overwhelms older Mac bots
  if 'MacMini4.1-GPU' in bot:
    blacklist.extend('_ image _ abnormal.wbmp'.split(' '))
    blacklist.extend(['msaa16', 'gm', '_', 'blurcircles'])

  match = []
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')

  if 'GalaxyS3' in bot:  # skia:1699
    match.append('~WritePixels')

  if 'AndroidOne' in bot:  # skia:4711
    match.append('~WritePixels')

  if 'NexusPlayer' in bot:
    match.append('~ResourceCache')

  if 'GalaxyS4' in bot:  # skia:4079
    match.append('~imagefiltersclipped')
    match.append('~imagefilterscropexpand')
    match.append('~scaled_tilemodes_npot')
    match.append('~bleed_image')  # skia:4367
    match.append('~ReadPixels')  # skia:4368

  if 'ANGLE' in bot and 'Debug' in bot:
    match.append('~GLPrograms') # skia:4717

  if 'MSAN' in bot:
    match.extend(['~Once', '~Shared'])  # Not sure what's up with these tests.

  if 'TSAN' in bot:
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
cov_end = lineno()   # Don't care about code coverage past here.


def self_test():
  import coverage  # This way the bots don't need coverage.py to be installed.
  args = {}
  cases = [
    'Pretend-iOS-Bot',
    'Test-Android-GCC-AndroidOne-GPU-Mali400MP2-Arm7-Release',
    'Test-Android-GCC-GalaxyS3-GPU-Mali400-Arm7-Debug',
    'Test-Android-GCC-GalaxyS4-GPU-SGX544-Arm7-Release',
    'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Release',
    'Test-Android-GCC-Nexus9-GPU-TegraK1-Arm64-Debug',
    'Test-Android-GCC-NexusPlayer-CPU-SSSE3-x86-Release',
    'Test-Android-GCC-NVIDIA_Shield-GPU-TegraX1-Arm64-Release',
    'Test-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Release',
    'Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-CommandBuffer',
    'Test-Mac10.8-Clang-MacMini4.1-CPU-SSE4-x86_64-Release',
    'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-MSAN',
    'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-TSAN',
    'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Valgrind',
    'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
    'Test-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug',
    'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-Vulkan',
    'Test-Win7-MSVC-ShuttleA-GPU-HD2000-x86-Debug-ANGLE',
  ]

  cov = coverage.coverage()
  cov.start()
  for case in cases:
    args[case] = get_args(case)
  cov.stop()

  this_file = os.path.basename(__file__)
  _, _, not_run, _ = cov.analysis(this_file)
  filtered = [line for line in not_run if line > cov_start and line < cov_end]
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
    json.dump(get_args(sys.argv[2]), out)
