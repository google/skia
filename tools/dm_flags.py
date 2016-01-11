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

  configs = ['565', '8888', 'gpu']

  if 'Android' not in bot:
    configs.extend(('upright-matrix-8888', 'upright-matrix-gpu'))
    args.extend('--matrix 0 1 1 0'.split(' '))

  if '-GCE-' in bot:
    configs.append('sp-8888')
    configs.extend(['twice-8888', '2ndpic-8888'])
    configs.extend(['remote-8888', 'remote_cache-8888'])

  if '-TSAN' not in bot:
    if ('TegraK1'  in bot or
        'GTX550Ti' in bot or
        'GTX660'   in bot or
        'GT610'    in bot):
      if 'Android' in bot:
        configs.append('nvprmsaa4')
      else:
        configs.append('nvprmsaa16')

  # The S4 crashes and the NP produces a long error stream when we run with
  # MSAA.  The Tegra2 and Tegra3 just don't support it.
  if ('GalaxyS4'    not in bot and
      'NexusPlayer' not in bot and
      'Tegra3'      not in bot and
      'iOS'         not in bot):
    if 'Android' in bot:
      configs.append('msaa4')
    else:
      configs.append('msaa16')
  # Runs out of memory on Android bots and Daisy.  Everyone else seems fine.
  if 'Android' not in bot and 'Daisy' not in bot:
    configs.append('pdf')
    configs.append('pdf_poppler')

  # NP is running out of RAM when we run all these modes.  skia:3255
  if 'NexusPlayer' not in bot:
    configs.extend(mode + '-8888' for mode in
                   ['serialize', 'tiles_rt', 'pic'])

  if 'ANGLE' in bot:
    configs.append('angle')

  # We want to run gpudft on atleast the mali 400
  if 'GalaxyS3' in bot:
      configs.append('gpudft')

  args.append('--config')
  args.extend(configs)

  # Run tests and gms everywhere,
  # and image decoding tests everywhere except GPU bots.
  # TODO: remove skp from default --src list?
  if 'GPU' in bot:
    args.extend('--src tests gm'.split(' '))
  else:
    args.extend('--src tests gm image'.split(' '))

  if 'GalaxyS' in bot:
    args.extend(('--threads', '0'))

  blacklist = []

  # Several of the newest version bmps fail on SkImageDecoder
  blacklist.extend('_ image decode pal8os2v2.bmp'.split(' '))
  blacklist.extend('_ image decode pal8v4.bmp'.split(' '))
  blacklist.extend('_ image decode pal8v5.bmp'.split(' '))
  blacklist.extend('_ image decode rgb16-565.bmp'.split(' '))
  blacklist.extend('_ image decode rgb16-565pal.bmp'.split(' '))
  blacklist.extend('_ image decode rgb32-111110.bmp'.split(' '))
  blacklist.extend('_ image decode rgb32bf.bmp'.split(' '))
  blacklist.extend('_ image decode rgba32.bmp'.split(' '))
  blacklist.extend('_ image decode rgba32abf.bmp'.split(' '))
  blacklist.extend('_ image decode rgb24largepal.bmp'.split(' '))
  blacklist.extend('_ image decode pal8os2v2-16.bmp'.split(' '))
  blacklist.extend('_ image decode pal8oversizepal.bmp'.split(' '))
  blacklist.extend('_ image decode pal4rletrns.bmp'.split(' '))
  blacklist.extend('_ image decode pal8rletrns.bmp'.split(' '))
  blacklist.extend('_ image decode 4bpp-pixeldata-cropped.bmp'.split(' '))
  blacklist.extend('_ image decode 8bpp-pixeldata-cropped.bmp'.split(' '))
  blacklist.extend('_ image decode 24bpp-pixeldata-cropped.bmp'.split(' '))
  blacklist.extend('_ image decode 32bpp-pixeldata-cropped.bmp'.split(' '))
  blacklist.extend('_ image decode testcase7.bmp'.split(' '))

  # New ico files that fail on SkImageDecoder
  blacklist.extend('_ image decode Hopstarter-Mac-Folders-Apple.ico'.split(' '))

  # Gif test image uses uninitialized memory on Mac bots
  if 'Mac' in bot:
    blacklist.extend('_ image decode frame_larger_than_image.gif'.split(' '))

  # Incomplete image tests that fail on SkImageDecoder
  blacklist.extend('_ image decode inc0.gif'.split(' '))
  blacklist.extend('_ image decode inc1.gif'.split(' '))
  blacklist.extend('_ image decode incInterlaced.gif'.split(' '))
  blacklist.extend('_ image decode inc0.jpg'.split(' '))
  blacklist.extend('_ image decode incGray.jpg'.split(' '))
  blacklist.extend('_ image decode inc0.wbmp'.split(' '))
  blacklist.extend('_ image decode inc1.wbmp'.split(' '))
  blacklist.extend('_ image decode inc0.webp'.split(' '))
  blacklist.extend('_ image decode inc1.webp'.split(' '))
  blacklist.extend('_ image decode inc0.ico'.split(' '))
  blacklist.extend('_ image decode inc1.ico'.split(' '))
  blacklist.extend('_ image decode inc0.png'.split(' '))
  blacklist.extend('_ image decode inc1.png'.split(' '))
  blacklist.extend('_ image decode inc2.png'.split(' '))
  blacklist.extend('_ image decode inc12.png'.split(' '))
  blacklist.extend('_ image decode inc13.png'.split(' '))
  blacklist.extend('_ image decode inc14.png'.split(' '))

  # Leon doesn't care about this, so why run it?
  if 'Win' in bot:
    blacklist.extend('_ image decode _'.split(' '))

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
    blacklist.extend('gpu image decode _ msaa image decode _'.split(' '))
    blacklist.extend('msaa16 gm _ tilemodesProcess'.split(' '))

  # the 32-bit GCE bots run out of memory in DM when running these large images
  if 'x86' in bot and not 'x86-64' in bot:
    blacklist.extend('_ image _ interlaced1.png'.split(' '))
    blacklist.extend('_ image _ interlaced2.png'.split(' '))
    blacklist.extend('_ image _ interlaced3.png'.split(' '))

  # skia:4095
  for test in ['not_native32_bitmap_config',
               'bleed_image',
               'bleed_alpha_image',
               'bleed_alpha_image_shader',
               'blend',
               'c_gms',
               'colortype',
               'colortype_xfermodes',
               'colorwheelnative',
               'drawfilter',
               'fontmgr_bounds_0.75_0',
               'fontmgr_bounds_1_-0.25',
               'fontmgr_bounds',
               'fontmgr_match',
               'fontmgr_iter',
               'lightingshader',
               'localmatriximagefilter',
               'path_stroke_with_zero_length',
               'textblobgeometrychange',
               'verylargebitmap',              # Windows only.
               'verylarge_picture_image']:     # Windows only.
    blacklist.extend(['serialize-8888', 'gm', '_', test])
  # skia:4769
  for test in ['blend',
               'drawfilter',
               'path_stroke_with_zero_length',
               'textblobgeometrychange']:
    blacklist.extend([    'sp-8888', 'gm', '_', test])
    blacklist.extend([   'pic-8888', 'gm', '_', test])
    blacklist.extend(['2ndpic-8888', 'gm', '_', test])
  for test in ['patch_primitive']:
    blacklist.extend(['sp-8888', 'gm', '_', test])
  # skia:4703
  for test in ['image-cacherator-from-picture',
               'image-cacherator-from-raster',
               'image-cacherator-from-ctable']:
    blacklist.extend([       'sp-8888', 'gm', '_', test])
    blacklist.extend([      'pic-8888', 'gm', '_', test])
    blacklist.extend([   '2ndpic-8888', 'gm', '_', test])
    blacklist.extend(['serialize-8888', 'gm', '_', test])

  if blacklist:
    args.append('--blacklist')
    args.extend(blacklist)

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

  if match:
    args.append('--match')
    args.extend(match)

  return args
cov_end = lineno()   # Don't care about code coverage past here.


def self_test():
  import coverage  # This way the bots don't need coverage.py to be installed.
  args = {}
  cases = [
    'Pretend-iOS-Bot',
    'Test-Android-GCC-AndroidOne-GPU-Mali400MP2-Arm7-Release',
    'Test-Android-GCC-Nexus9-GPU-TegraK1-Arm64-Debug',
    'Test-Android-GCC-GalaxyS3-GPU-Mali400-Arm7-Debug',
    'Test-Android-GCC-GalaxyS4-GPU-SGX544-Arm7-Release',
    'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Release',
    'Test-Android-GCC-NexusPlayer-CPU-SSSE3-x86-Release',
    'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
    'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-TSAN',
    'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Valgrind',
    'Test-Win7-MSVC-ShuttleA-GPU-HD2000-x86-Debug-ANGLE',
    'Test-Mac10.8-Clang-MacMini4.1-CPU-SSE4-x86_64-Release',
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
