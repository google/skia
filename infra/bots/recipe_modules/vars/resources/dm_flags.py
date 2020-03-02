# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Generate flags for DM.
# TODO(borenet): This is sitting in the vars recipe module as a hacky way to
# ensure that it's available in the recipe bundle while also making it easily
# usable in gen_tasks. The ultimate goal is to move this into gen_tasks itself.


import argparse
import json
import sys


def key_params(parts):
  """Build a unique key from the builder name (as a list).

  E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
  """
  # Don't bother to include role, which is always Test.
  blacklist = ['role', 'test_filter']

  flat = []
  for k in sorted(parts.keys()):
    if k not in blacklist:
      flat.append(k)
      flat.append(parts[k])

  return flat


def dm_flags(bot, parts, task_id, revision, issue, patchset, patch_storage,
             buildbucket_build_id, swarming_bot_id, swarming_task_id,
             internal_hardware_label):
  # TODO(borenet): This duplicates code in recipes_modules/vars/api.py and will
  # be removed soon.
  is_linux = (
        'Ubuntu' in bot
     or 'Debian' in bot
     or 'Housekeeper' in bot
  )
  extra_tokens = []
  if len(parts.get('extra_config', '')) > 0:
    if parts['extra_config'].startswith('SK'):
      assert parts['extra_config'].isupper()
      extra_tokens = [parts['extra_config']]
    else:
      extra_tokens = parts['extra_config'].split('_')

  properties = {
    'gitHash':              revision,
    'builder':              bot,
    'buildbucket_build_id': buildbucket_build_id,
    'task_id':              task_id,
    'issue':                issue,
    'patchset':             patchset,
    'patch_storage':        patch_storage,
    'swarming_bot_id':      swarming_bot_id,
    'swarming_task_id':     swarming_task_id,
  }

  args = [
    'dm',
    '--nameByHash',
  ]

  args.append('--key')
  keys = key_params(parts)

  if 'Lottie' in parts.get('extra_config', ''):
    keys.extend(['renderer', 'skottie'])
  if 'DDL' in parts.get('extra_config', ''):
    # 'DDL' style means "--skpViewportSize 2048 --pr ~small"
    keys.extend(['style', 'DDL'])
  else:
    keys.extend(['style', 'default'])

  args.extend(keys)

  configs = []
  blacklisted = []

  def blacklist(quad):
    config, src, options, name = (
        quad.split(' ') if isinstance(quad, str) else quad)
    if (config == '_' or
        config in configs or
        (config[0] == '~' and config[1:] in configs)):
      blacklisted.extend([config, src, options, name])

  # This enables non-deterministic random seeding of the GPU FP optimization
  # test.
  # Not Android due to:
  #  - https://skia.googlesource.com/skia/+/
  #    5910ed347a638ded8cd4c06dbfda086695df1112/BUILD.gn#160
  #  - https://skia.googlesource.com/skia/+/
  #    ce06e261e68848ae21cac1052abc16bc07b961bf/tests/ProcessorTest.cpp#307
  # Not MSAN due to:
  #  - https://skia.googlesource.com/skia/+/
  #    0ac06e47269a40c177747310a613d213c95d1d6d/infra/bots/recipe_modules/
  #    flavor/gn_flavor.py#80
  if 'Android' not in bot and 'MSAN' not in bot:
    args.append('--randomProcessorTest')

  if 'Pixel3' in bot and 'Vulkan' in bot:
    args.extend(['--dontReduceOpsTaskSplitting'])

  thread_limit = None
  MAIN_THREAD_ONLY = 0

  # 32-bit desktop bots tend to run out of memory, because they have relatively
  # far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
  if '-x86-' in bot:
    thread_limit = 4

  # These bots run out of memory easily.
  if 'MotoG4' in bot or 'Nexus7' in bot:
    thread_limit = MAIN_THREAD_ONLY

  # Avoid issues with dynamically exceeding resource cache limits.
  if 'Test' in bot and 'DISCARDABLE' in bot:
    thread_limit = MAIN_THREAD_ONLY

  if thread_limit is not None:
    args.extend(['--threads', str(thread_limit)])

  if 'SwiftShader' in extra_tokens:
    configs.extend(['gles', 'glesdft'])
    args.append('--disableDriverCorrectnessWorkarounds')

  elif parts.get('cpu_or_gpu') == 'CPU':
    args.append('--nogpu')

    configs.append('8888')

    if 'BonusConfigs' in bot:
      configs = [
        'g8', '565',
        'pic-8888', 'serialize-8888',
        'f16', 'srgb', 'esrgb', 'narrow', 'enarrow',
        'p3', 'ep3', 'rec2020', 'erec2020']

    if 'PDF' in bot:
      configs = [ 'pdf' ]
      args.append('--rasterize_pdf')  # Works only on Mac.
      # Take ~forever to rasterize:
      blacklist('pdf gm _ lattice2')
      blacklist('pdf gm _ hairmodes')
      blacklist('pdf gm _ longpathdash')

  elif parts.get('cpu_or_gpu') == 'GPU':
    args.append('--nocpu')

    # Add in either gles or gl configs to the canonical set based on OS
    sample_count = '8'
    gl_prefix = 'gl'
    if 'Android' in bot or 'iOS' in bot:
      sample_count = '4'
      # We want to test the OpenGL config not the GLES config on the Shield
      if 'NVIDIA_Shield' not in bot:
        gl_prefix = 'gles'
      # MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
      if ('Pixel3a' in bot):
        sample_count = ''
    elif 'Intel' in bot:
      # MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
      sample_count = ''
    elif 'ChromeOS' in bot:
      gl_prefix = 'gles'

    if 'NativeFonts' in bot:
      configs.append(gl_prefix)
    else:
      configs.extend([gl_prefix,
                      gl_prefix + 'dft',
                      gl_prefix + 'srgb'])
      if sample_count:
        configs.append(gl_prefix + 'msaa' + sample_count)

    # The Tegra3 doesn't support MSAA
    if ('Tegra3'      in bot or
        # We aren't interested in fixing msaa bugs on current iOS devices.
        'iPad4' in bot or
        'iPadPro' in bot or
        'iPhone6' in bot or
        'iPhone7' in bot or
        # skia:5792
        'IntelHD530'   in bot or
        'IntelIris540' in bot):
      configs = [x for x in configs if 'msaa' not in x]

    # We want to test both the OpenGL config and the GLES config on Linux Intel:
    # GL is used by Chrome, GLES is used by ChromeOS.
    # Also do the Ganesh threading verification test (render with and without
    # worker threads, using only the SW path renderer, and compare the results).
    if 'Intel' in bot and is_linux:
      configs.extend(['gles',
                      'glesdft',
                      'glessrgb',
                      'gltestthreading'])
      # skbug.com/6333, skbug.com/6419, skbug.com/6702
      blacklist('gltestthreading gm _ lcdblendmodes')
      blacklist('gltestthreading gm _ lcdoverlap')
      blacklist('gltestthreading gm _ textbloblooper')
      # All of these GMs are flaky, too:
      blacklist('gltestthreading gm _ bleed_alpha_bmp')
      blacklist('gltestthreading gm _ bleed_alpha_bmp_shader')
      blacklist('gltestthreading gm _ bleed_alpha_image')
      blacklist('gltestthreading gm _ bleed_alpha_image_shader')
      blacklist('gltestthreading gm _ savelayer_with_backdrop')
      blacklist('gltestthreading gm _ persp_shaders_bw')
      blacklist('gltestthreading gm _ dftext_blob_persp')
      blacklist('gltestthreading gm _ dftext')
      # skbug.com/7523 - Flaky on various GPUs
      blacklist('gltestthreading gm _ orientation')
      # These GMs only differ in the low bits
      blacklist('gltestthreading gm _ stroketext')
      blacklist('gltestthreading gm _ draw_image_set')

    # CommandBuffer bot *only* runs the command_buffer config.
    if 'CommandBuffer' in bot:
      configs = ['commandbuffer']

    # ANGLE bot *only* runs the angle configs
    if 'ANGLE' in bot:
      configs = ['angle_d3d11_es2',
                 'angle_d3d9_es2',
                 'angle_gl_es2',
                 'angle_d3d11_es3']
      if sample_count:
        configs.append('angle_d3d11_es2_msaa' + sample_count)
        configs.append('angle_d3d11_es3_msaa' + sample_count)
      if 'LenovoYogaC630' in bot:
        # LenovoYogaC630 only supports D3D11, and to save time, we only test ES3
        configs = ['angle_d3d11_es3',
                   'angle_d3d11_es3_msaa' + sample_count]
      if 'GTX' in bot or 'Quadro' in bot:
        # See skia:7823 and chromium:693090.
        configs.append('angle_gl_es3')
        if sample_count:
          configs.append('angle_gl_es2_msaa' + sample_count)
          configs.append('angle_gl_es3_msaa' + sample_count)
      if 'NUC5i7RYH' in bot:
        # skbug.com/7376
        blacklist('_ test _ ProcessorCloneTest')

    if 'AndroidOne' in bot or ('Nexus' in bot and 'Nexus5x' not in bot) or 'GalaxyS6' in bot:
      # skbug.com/9019
      blacklist('_ test _ ProcessorCloneTest')
      blacklist('_ test _ Programs')
      blacklist('_ test _ ProcessorOptimizationValidationTest')

    if 'CommandBuffer' in bot and 'MacBook10.1-' in bot:
      # skbug.com/9235
      blacklist('_ test _ Programs')

    # skbug.com/9033 - these devices run out of memory on this test
    # when opList splitting reduction is enabled
    if 'GPU' in bot and ('Nexus7' in bot or
                         'NVIDIA_Shield' in bot or
                         'Nexus5x' in bot or
                         ('Win10' in bot and 'GTX660' in bot and 'Vulkan' in bot)):
      blacklist(['_', 'gm', '_', 'savelayer_clipmask'])

    # skbug.com/9123
    if 'CommandBuffer' in bot and 'IntelIris5100' in bot:
      blacklist(['_', 'test', '_', 'AsyncReadPixels'])

    # skbug.com/9043 - these devices render this test incorrectly
    # when opList splitting reduction is enabled
    if 'GPU' in bot and 'Vulkan' in bot and ('RadeonR9M470X' in bot or
                                             'RadeonHD7770' in bot):
      blacklist(['_', 'tests', '_', 'VkDrawableImportTest'])

    if 'Vulkan' in bot:
      configs = ['vk']
      if 'Android' in bot:
        configs.append('vkmsaa4')
      else:
        # MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skia:9023
        if 'Intel' not in bot:
          configs.append('vkmsaa8')

    if 'Metal' in bot:
      configs = ['mtl']
      if 'iOS' in bot:
        configs.append('mtlmsaa4')
      else:
        # MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
        if 'Intel' not in bot:
          configs.append('mtlmsaa8')

    # Test 1010102 on our Linux/NVIDIA bots and the persistent cache config
    # on the GL bots.
    if ('QuadroP400' in bot and 'PreAbandonGpuContext' not in bot and
        'TSAN' not in bot and is_linux):
      if 'Vulkan' in bot:
        configs.append('vk1010102')
        # Decoding transparent images to 1010102 just looks bad
        blacklist('vk1010102 image _ _')
      else:
        configs.extend(['gl1010102',
                        'gltestpersistentcache',
                        'gltestglslcache',
                        'gltestprecompile'])
        # Decoding transparent images to 1010102 just looks bad
        blacklist('gl1010102 image _ _')
        # These tests produce slightly different pixels run to run on NV.
        blacklist('gltestpersistentcache gm _ atlastext')
        blacklist('gltestpersistentcache gm _ dftext')
        blacklist('gltestpersistentcache gm _ glyph_pos_h_b')
        blacklist('gltestglslcache gm _ atlastext')
        blacklist('gltestglslcache gm _ dftext')
        blacklist('gltestglslcache gm _ glyph_pos_h_b')
        blacklist('gltestprecompile gm _ atlastext')
        blacklist('gltestprecompile gm _ dftext')
        blacklist('gltestprecompile gm _ glyph_pos_h_b')
        # Tessellation shaders do not yet participate in the persistent cache.
        blacklist('gltestpersistentcache gm _ tessellation')
        blacklist('gltestglslcache gm _ tessellation')
        blacklist('gltestprecompile gm _ tessellation')

    # We also test the SkSL precompile config on Pixel2XL as a representative
    # Android device - this feature is primarily used by Flutter.
    if 'Pixel2XL' in bot and 'Vulkan' not in bot:
      configs.append('glestestprecompile')

    # Test rendering to wrapped dsts on a few bots
    # Also test 'glenarrow', which hits F16 surfaces and F16 vertex colors.
    if 'BonusConfigs' in extra_tokens:
      configs = ['glbetex', 'glbert', 'glenarrow']


    if 'ChromeOS' in bot:
      # Just run GLES for now - maybe add gles_msaa4 in the future
      configs = ['gles']

    # Test coverage counting path renderer.
    if 'CCPR' in bot:
      configs = [c for c in configs if c == 'gl' or c == 'gles']
      args.extend(['--pr', 'ccpr', '--cc', 'true', '--cachePathMasks', 'false'])

    # Test GPU tessellation path renderer.
    if 'GpuTess' in bot:
      configs = [gl_prefix + 'msaa4']
      args.extend(['--pr', 'gtess'])

    # Test non-nvpr on NVIDIA.
    if 'NonNVPR' in bot:
      configs = ['gl', 'glmsaa4']
      args.extend(['--pr', '~nvpr'])

    # DDL is a GPU-only feature
    if 'DDL1' in bot:
      # This bot generates gl and vk comparison images for the large skps
      configs = [c for c in configs if c == 'gl' or c == 'vk' or c == 'mtl']
      args.extend(['--skpViewportSize', "2048"])
      args.extend(['--pr', '~small'])
    if 'DDL3' in bot:
      # This bot generates the ddl-gl and ddl-vk images for the
      # large skps and the gms
      ddl_configs = ['ddl-' + c for c in configs if c == 'gl' or c == 'vk' or c == 'mtl']
      ddl2_configs = ['ddl2-' + c for c in configs if c == 'gl' or c == 'vk' or c == 'mtl']
      configs = ddl_configs + ddl2_configs
      args.extend(['--skpViewportSize', "2048"])
      args.extend(['--gpuThreads', "0"])

  tf = parts.get('test_filter')
  if 'All' != tf:
    # Expected format: shard_XX_YY
    split = tf.split('_')
    if len(split) == 3:
      args.extend(['--shard', split[1]])
      args.extend(['--shards', split[2]])
    else: # pragma: nocover
      raise Exception('Invalid task name - bad shards: %s' % tf)

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image lottie colorImage svg skp'.split(' '))
  if parts.get('cpu_or_gpu') == 'GPU':
    # Don't run the 'svgparse_*' svgs on GPU.
    blacklist('_ svg _ svgparse_')
  elif bot == 'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN':
    # Only run the CPU SVGs on 8888.
    blacklist('~8888 svg _ _')
  else:
    # On CPU SVGs we only care about parsing. Only run them on the above bot.
    args.remove('svg')

  # Eventually I'd like these to pass, but for now just skip 'em.
  if 'SK_FORCE_RASTER_PIPELINE_BLITTER' in bot:
    args.remove('tests')

  if 'NativeFonts' in bot:  # images won't exercise native font integration :)
    args.remove('image')
    args.remove('colorImage')

  def remove_from_args(arg):
    if arg in args:
      args.remove(arg)

  if 'DDL' in bot or 'PDF' in bot:
    # The DDL and PDF bots just render the large skps and the gms
    remove_from_args('tests')
    remove_from_args('image')
    remove_from_args('colorImage')
    remove_from_args('svg')
  else:
    # No other bots render the .skps.
    remove_from_args('skp')

  if 'Lottie' in parts.get('extra_config', ''):
    # Only run the lotties on Lottie bots.
    remove_from_args('tests')
    remove_from_args('gm')
    remove_from_args('image')
    remove_from_args('colorImage')
    remove_from_args('svg')
    remove_from_args('skp')
  else:
    remove_from_args('lottie')

  # TODO: ???
  blacklist('f16 _ _ dstreadshuffle')
  blacklist('glsrgb image _ _')
  blacklist('glessrgb image _ _')

  # --src image --config g8 means "decode into Gray8", which isn't supported.
  blacklist('g8 image _ _')
  blacklist('g8 colorImage _ _')

  if 'Valgrind' in bot:
    # These take 18+ hours to run.
    blacklist('pdf gm _ fontmgr_iter')
    blacklist('pdf _ _ PANO_20121023_214540.jpg')
    blacklist('pdf skp _ worldjournal')
    blacklist('pdf skp _ desk_baidu.skp')
    blacklist('pdf skp _ desk_wikipedia.skp')
    blacklist('_ svg _ _')
    # skbug.com/9171 and 8847
    blacklist('_ test _ InitialTextureClear')

  if 'TecnoSpark3Pro' in bot:
    # skbug.com/9421
    blacklist('_ test _ InitialTextureClear')

  if 'iOS' in bot:
    blacklist(gl_prefix + ' skp _ _')

  if 'Mac' in bot or 'iOS' in bot:
    # CG fails on questionable bmps
    blacklist('_ image gen_platf rgba32abf.bmp')
    blacklist('_ image gen_platf rgb24prof.bmp')
    blacklist('_ image gen_platf rgb24lprof.bmp')
    blacklist('_ image gen_platf 8bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 4bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 32bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 24bpp-pixeldata-cropped.bmp')

    # CG has unpredictable behavior on this questionable gif
    # It's probably using uninitialized memory
    blacklist('_ image gen_platf frame_larger_than_image.gif')

    # CG has unpredictable behavior on incomplete pngs
    # skbug.com/5774
    blacklist('_ image gen_platf inc0.png')
    blacklist('_ image gen_platf inc1.png')
    blacklist('_ image gen_platf inc2.png')
    blacklist('_ image gen_platf inc3.png')
    blacklist('_ image gen_platf inc4.png')
    blacklist('_ image gen_platf inc5.png')
    blacklist('_ image gen_platf inc6.png')
    blacklist('_ image gen_platf inc7.png')
    blacklist('_ image gen_platf inc8.png')
    blacklist('_ image gen_platf inc9.png')
    blacklist('_ image gen_platf inc10.png')
    blacklist('_ image gen_platf inc11.png')
    blacklist('_ image gen_platf inc12.png')
    blacklist('_ image gen_platf inc13.png')
    blacklist('_ image gen_platf inc14.png')
    blacklist('_ image gen_platf incInterlaced.png')

    # These images fail after Mac 10.13.1 upgrade.
    blacklist('_ image gen_platf incInterlaced.gif')
    blacklist('_ image gen_platf inc1.gif')
    blacklist('_ image gen_platf inc0.gif')
    blacklist('_ image gen_platf butterfly.gif')

  # WIC fails on questionable bmps
  if 'Win' in bot:
    blacklist('_ image gen_platf pal8os2v2.bmp')
    blacklist('_ image gen_platf pal8os2v2-16.bmp')
    blacklist('_ image gen_platf rgba32abf.bmp')
    blacklist('_ image gen_platf rgb24prof.bmp')
    blacklist('_ image gen_platf rgb24lprof.bmp')
    blacklist('_ image gen_platf 8bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 4bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 32bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 24bpp-pixeldata-cropped.bmp')
    if 'x86_64' in bot and 'CPU' in bot:
      # This GM triggers a SkSmallAllocator assert.
      blacklist('_ gm _ composeshader_bitmap')

  if 'Win' in bot or 'Mac' in bot:
    # WIC and CG fail on arithmetic jpegs
    blacklist('_ image gen_platf testimgari.jpg')
    # More questionable bmps that fail on Mac, too. skbug.com/6984
    blacklist('_ image gen_platf rle8-height-negative.bmp')
    blacklist('_ image gen_platf rle4-height-negative.bmp')

  # These PNGs have CRC errors. The platform generators seem to draw
  # uninitialized memory without reporting an error, so skip them to
  # avoid lots of images on Gold.
  blacklist('_ image gen_platf error')

  if 'Android' in bot or 'iOS' in bot:
    # This test crashes the N9 (perhaps because of large malloc/frees). It also
    # is fairly slow and not platform-specific. So we just disable it on all of
    # Android and iOS. skia:5438
    blacklist('_ test _ GrShape')

  if internal_hardware_label == '5':
    # http://b/118312149#comment9
    blacklist('_ test _ SRGBReadWritePixels')

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
                       'fontmgr_iter',
                       'imagemasksubset',
                       'wacky_yuv_formats_domain',
                       'imagemakewithfilter',
                       'imagemakewithfilter_crop',
                       'imagemakewithfilter_crop_ref',
                       'imagemakewithfilter_ref']

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

  # skia:5778
  bad_serialize_gms.append('typefacerendering_pfaMac')
  # skia:5942
  bad_serialize_gms.append('parsedpaths')

  # these use a custom image generator which doesn't serialize
  bad_serialize_gms.append('ImageGeneratorExternal_rect')
  bad_serialize_gms.append('ImageGeneratorExternal_shader')

  # skia:6189
  bad_serialize_gms.append('shadow_utils')

  # skia:7938
  bad_serialize_gms.append('persp_images')

  # Not expected to round trip encoding/decoding.
  bad_serialize_gms.append('all_bitmap_configs')
  bad_serialize_gms.append('makecolorspace')
  bad_serialize_gms.append('readpixels')
  bad_serialize_gms.append('draw_image_set_rect_to_rect')
  bad_serialize_gms.append('compositor_quads_shader')
  bad_serialize_gms.append('wacky_yuv_formats_qtr')

  # This GM forces a path to be convex. That property doesn't survive
  # serialization.
  bad_serialize_gms.append('analytic_antialias_convex')

  for test in bad_serialize_gms:
    blacklist(['serialize-8888', 'gm', '_', test])

  if 'Mac' not in bot:
    for test in ['bleed_alpha_image', 'bleed_alpha_image_shader']:
      blacklist(['serialize-8888', 'gm', '_', test])
  # It looks like we skip these only for out-of-memory concerns.
  if 'Win' in bot or 'Android' in bot:
    for test in ['verylargebitmap', 'verylarge_picture_image']:
      blacklist(['serialize-8888', 'gm', '_', test])
  if 'Mac' in bot and 'CPU' in bot:
    # skia:6992
    blacklist(['pic-8888', 'gm', '_', 'encode-platform'])
    blacklist(['serialize-8888', 'gm', '_', 'encode-platform'])

  # skia:4769
  for test in ['drawfilter']:
    blacklist([   'pic-8888', 'gm', '_', test])
  # skia:4703
  for test in ['image-cacherator-from-picture',
               'image-cacherator-from-raster',
               'image-cacherator-from-ctable']:
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

  # GM that requires raster-backed canvas
  for test in ['complexclip4_bw', 'complexclip4_aa', 'p3',
               'async_rescale_and_read_text_up_large',
               'async_rescale_and_read_text_up',
               'async_rescale_and_read_text_down',
               'async_rescale_and_read_dog_up',
               'async_rescale_and_read_dog_down',
               'async_rescale_and_read_rose',
               'async_rescale_and_read_no_bleed']:
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

  # GM requires canvas->makeSurface() to return a valid surface.
    blacklist([      'pic-8888', 'gm', '_', "blurrect_compare"])
    blacklist(['serialize-8888', 'gm', '_', "blurrect_compare"])

  # Extensions for RAW images
  r = ['arw', 'cr2', 'dng', 'nef', 'nrw', 'orf', 'raf', 'rw2', 'pef', 'srw',
       'ARW', 'CR2', 'DNG', 'NEF', 'NRW', 'ORF', 'RAF', 'RW2', 'PEF', 'SRW']

  # skbug.com/4888
  # Blacklist RAW images (and a few large PNGs) on GPU bots
  # until we can resolve failures.
  if 'GPU' in bot:
    blacklist('_ image _ interlaced1.png')
    blacklist('_ image _ interlaced2.png')
    blacklist('_ image _ interlaced3.png')
    for raw_ext in r:
      blacklist('_ image _ .%s' % raw_ext)

  # Blacklist memory intensive tests on 32-bit bots.
  if 'Win8' in bot and 'x86-' in bot:
    blacklist('_ image f16 _')
    blacklist('_ image _ abnormal.wbmp')
    blacklist('_ image _ interlaced1.png')
    blacklist('_ image _ interlaced2.png')
    blacklist('_ image _ interlaced3.png')
    for raw_ext in r:
      blacklist('_ image _ .%s' % raw_ext)

  if 'Nexus5' in bot and 'GPU' in bot:
    # skia:5876
    blacklist(['_', 'gm', '_', 'encode-platform'])

  if 'AndroidOne-GPU' in bot:  # skia:4697, skia:4704, skia:4694, skia:4705
    blacklist(['_',            'gm', '_', 'bigblurs'])
    blacklist(['_',            'gm', '_', 'bleed'])
    blacklist(['_',            'gm', '_', 'bleed_alpha_bmp'])
    blacklist(['_',            'gm', '_', 'bleed_alpha_bmp_shader'])
    blacklist(['_',            'gm', '_', 'bleed_alpha_image'])
    blacklist(['_',            'gm', '_', 'bleed_alpha_image_shader'])
    blacklist(['_',            'gm', '_', 'bleed_image'])
    blacklist(['_',            'gm', '_', 'dropshadowimagefilter'])
    blacklist(['_',            'gm', '_', 'filterfastbounds'])
    blacklist([gl_prefix,      'gm', '_', 'imageblurtiled'])
    blacklist(['_',            'gm', '_', 'imagefiltersclipped'])
    blacklist(['_',            'gm', '_', 'imagefiltersscaled'])
    blacklist(['_',            'gm', '_', 'imageresizetiled'])
    blacklist(['_',            'gm', '_', 'matrixconvolution'])
    blacklist(['_',            'gm', '_', 'strokedlines'])
    if sample_count:
      gl_msaa_config = gl_prefix + 'msaa' + sample_count
      blacklist([gl_msaa_config, 'gm', '_', 'imageblurtiled'])
      blacklist([gl_msaa_config, 'gm', '_', 'imagefiltersbase'])

  match = []
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')

  if 'Valgrind' in bot and 'PreAbandonGpuContext' in bot:
    # skia:6575
    match.append('~multipicturedraw_')

  if 'AndroidOne' in bot:
    match.append('~WritePixels')  # skia:4711
    match.append('~PremulAlphaRoundTrip_Gpu')  # skia:7501
    match.append('~ReimportImageTextureWithMipLevels')  # skia:8090

  if 'GalaxyS6' in bot:
    match.append('~SpecialImage') # skia:6338
    match.append('~skbug6653') # skia:6653

  if 'MSAN' in bot:
    match.extend(['~Once', '~Shared'])  # Not sure what's up with these tests.

  if 'TSAN' in bot:
    match.extend(['~ReadWriteAlpha'])   # Flaky on TSAN-covered on nvidia bots.
    match.extend(['~RGBA4444TextureTest',  # Flakier than they are important.
                  '~RGB565TextureTest'])

  # By default, we test with GPU threading enabled, unless specifically
  # disabled.
  if 'NoGPUThreads' in bot:
    args.extend(['--gpuThreads', '0'])

  if 'Vulkan' in bot and 'Adreno530' in bot:
      # skia:5777
      match.extend(['~CopySurface'])

  if 'Vulkan' in bot and 'Adreno' in bot:
      # skia:7663
      match.extend(['~WritePixelsNonTextureMSAA_Gpu'])
      match.extend(['~WritePixelsMSAA_Gpu'])

  if 'Vulkan' in bot and is_linux and 'IntelIris640' in bot:
    match.extend(['~VkHeapTests']) # skia:6245

  if is_linux and 'IntelIris640' in bot:
    match.extend(['~Programs']) # skia:7849

  if 'TecnoSpark3Pro' in bot:
    match.extend(['~Programs']) # skia:9814

  if 'IntelIris640' in bot or 'IntelHD615' in bot or 'IntelHDGraphics615' in bot:
    match.append('~^SRGBReadWritePixels$') # skia:9225

  if 'Vulkan' in bot and is_linux and 'IntelHD405' in bot:
    # skia:7322
    blacklist(['vk', 'gm', '_', 'skbug_257'])
    blacklist(['vk', 'gm', '_', 'filltypespersp'])
    match.append('~^ClearOp$')
    match.append('~^CopySurface$')
    match.append('~^ImageNewShader_GPU$')
    match.append('~^InitialTextureClear$')
    match.append('~^PinnedImageTest$')
    match.append('~^ReadPixels_Gpu$')
    match.append('~^ReadPixels_Texture$')
    match.append('~^SRGBReadWritePixels$')
    match.append('~^VkUploadPixelsTests$')
    match.append('~^WritePixelsNonTexture_Gpu$')
    match.append('~^WritePixelsNonTextureMSAA_Gpu$')
    match.append('~^WritePixels_Gpu$')
    match.append('~^WritePixelsMSAA_Gpu$')

  if 'Vulkan' in bot and 'GTX660' in bot and 'Win' in bot:
    # skbug.com/8047
    match.append('~FloatingPointTextureTest$')

  if 'Metal' in bot and 'HD8870M' in bot and 'Mac' in bot:
    # skia:9255
    match.append('~WritePixelsNonTextureMSAA_Gpu')

  if 'ANGLE' in bot:
    # skia:7835
    match.append('~BlurMaskBiggerThanDest')

  if 'IntelIris6100' in bot and 'ANGLE' in bot and 'Release' in bot:
    # skia:7376
    match.append('~^ProcessorOptimizationValidationTest$')

  if ('IntelIris6100' in bot or 'IntelHD4400' in bot) and 'ANGLE' in bot:
    # skia:6857
    blacklist(['angle_d3d9_es2', 'gm', '_', 'lighting'])

  if 'PowerVRGX6250' in bot:
    match.append('~gradients_view_perspective_nodither') #skia:6972

  if '-arm-' in bot and 'ASAN' in bot:
    # TODO: can we run with env allocator_may_return_null=1 instead?
    match.append('~BadImage')

  if 'Mac' in bot and 'IntelHD6000' in bot:
    # skia:7574
    match.append('~^ProcessorCloneTest$')
    match.append('~^GrMeshTest$')

  if 'Mac' in bot and 'IntelHD615' in bot:
    # skia:7603
    match.append('~^GrMeshTest$')

  if 'LenovoYogaC630' in bot and 'ANGLE' in extra_tokens:
    # skia:9275
    blacklist(['_', 'tests', '_', 'Programs'])
    # skia:8976
    blacklist(['_', 'tests', '_', 'GrDefaultPathRendererTest'])
    # https://bugs.chromium.org/p/angleproject/issues/detail?id=3414
    blacklist(['_', 'tests', '_', 'PinnedImageTest'])

  if blacklisted:
    args.append('--blacklist')
    args.extend(blacklisted)

  if match:
    args.append('--match')
    args.extend(match)

  # These bots run out of memory running RAW codec tests. Do not run them in
  # parallel
  if 'Nexus5' in bot or 'Nexus9' in bot:
    args.append('--noRAW_threading')

  if 'FSAA' in bot:
    args.extend(['--analyticAA', 'false'])
  if 'FAAA' in bot:
    args.extend(['--forceAnalyticAA'])

  if 'NativeFonts' not in bot:
    args.append('--nonativeFonts')

  if 'GDI' in bot:
    args.append('--gdi')

  # Let's make all bots produce verbose output by default.
  args.append('--verbose')

  # See skia:2789.
  if 'AbandonGpuContext' in extra_tokens:
    args.append('--abandonGpuContext')
  if 'PreAbandonGpuContext' in extra_tokens:
    args.append('--preAbandonGpuContext')
  if 'ReleaseAndAbandonGpuContext' in extra_tokens:
    args.append('--releaseAndAbandonGpuContext')

  return args, properties


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--bot', required=True)
  parser.add_argument('--parts', required=True)
  parser.add_argument('--task_id', required=True)
  parser.add_argument('--revision', required=True)
  parser.add_argument('--issue', required=True)
  parser.add_argument('--patchset', required=True)
  parser.add_argument('--patch_storage', required=True)
  parser.add_argument('--buildbucket_build_id', required=True)
  parser.add_argument('--swarming_bot_id', required=True)
  parser.add_argument('--swarming_task_id', required=True)
  parser.add_argument('--internal_hardware_label', required=True)
  args = parser.parse_args()
  args, props = dm_flags(
    bot=args.bot,
    parts=json.loads(args.parts),
    task_id=args.task_id,
    revision=args.revision,
    issue=args.issue,
    patchset=args.patchset,
    patch_storage=args.patch_storage,
    buildbucket_build_id=args.buildbucket_build_id,
    swarming_bot_id=args.swarming_bot_id,
    swarming_task_id=args.swarming_task_id,
    internal_hardware_label=args.internal_hardware_label,
  )
  print json.dumps({
      'dm_flags': args,
      'dm_properties': props,
  }, separators=(',', ':'))
