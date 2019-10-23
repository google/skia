# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


DEPS = [
  'env',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]


def upload_dm_results(buildername):
  skip_upload_bots = [
    'ASAN',
    'Coverage',
    'MSAN',
    'TSAN',
    'UBSAN',
    'Valgrind',
  ]
  for s in skip_upload_bots:
    if s in buildername:
      return False
  return True


def dm_flags(api, bot):
  args = []
  configs = []
  blacklisted = []

  def blacklist(quad):
    config, src, options, name = (
        quad.split(' ') if isinstance(quad, str) else quad)
    if (config == '_' or
        config in configs or
        (config[0] == '~' and config[1:] in configs)):
      blacklisted.extend([config, src, options, name])

  # We've been spending lots of time writing out and especially uploading
  # .pdfs, but not doing anything further with them.  skia:6821
  args.extend(['--dont_write', 'pdf'])

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
  if 'Chromecast' in bot or 'MotoG4' in bot or 'Nexus7' in bot:
    thread_limit = MAIN_THREAD_ONLY

  # Avoid issues with dynamically exceeding resource cache limits.
  if 'Test' in bot and 'DISCARDABLE' in bot:
    thread_limit = MAIN_THREAD_ONLY

  if thread_limit is not None:
    args.extend(['--threads', str(thread_limit)])

  # Android's kernel will occasionally attempt to kill our process, using
  # SIGINT, in an effort to free up resources. If requested, that signal
  # is ignored and dm will keep attempting to proceed until we actually
  # exhaust the available resources.
  if 'Chromecast' in bot:
    args.append('--ignoreSigInt')

  if 'SwiftShader' in api.vars.extra_tokens:
    configs.extend(['gles', 'glesdft'])
    args.append('--disableDriverCorrectnessWorkarounds')

  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    args.append('--nogpu')

    configs.append('8888')

    if 'BonusConfigs' in bot or ('SAN' in bot and 'GCE' in bot):
      configs.extend([
        'pdf',
        'g8', '565',
        'pic-8888', 'tiles_rt-8888', 'serialize-8888',
        'f16', 'srgb', 'esrgb', 'narrow', 'enarrow',
        'p3', 'ep3', 'rec2020', 'erec2020'])

  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    args.append('--nocpu')

    # Add in either gles or gl configs to the canonical set based on OS
    sample_count = '8'
    gl_prefix = 'gl'
    if 'Android' in bot or 'iOS' in bot:
      sample_count = '4'
      # We want to test the OpenGL config not the GLES config on the Shield
      if 'NVIDIA_Shield' not in bot:
        gl_prefix = 'gles'
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
    if 'Intel' in bot and api.vars.is_linux:
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
      blacklist('_ test _ GLPrograms')
      blacklist('_ test _ ProcessorOptimizationValidationTest')

    if 'CommandBuffer' in bot and 'MacBook10.1-' in bot:
      # skbug.com/9235
      blacklist('_ test _ GLPrograms')

    # skbug.com/9033 - these devices run out of memory on this test
    # when opList splitting reduction is enabled
    if 'GPU' in bot and ('Nexus7' in bot or
                         'NVIDIA_Shield' in bot or
                         'Nexus5x' in bot or
                         ('Win10' in bot and 'GTX660' in bot and 'Vulkan' in bot) or
                         'Chorizo' in bot):
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
        'TSAN' not in bot and api.vars.is_linux):
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

    # We also test the SkSL precompile config on Pixel2XL as a representative
    # Android device - this feature is primarily used by Flutter.
    if 'Pixel2XL' in bot and 'Vulkan' not in bot:
      configs.append('glestestprecompile')

    # Test rendering to wrapped dsts on a few bots
    # Also test 'glenarrow', which hits F16 surfaces and F16 vertex colors.
    if 'BonusConfigs' in api.vars.extra_tokens:
      configs = ['glbetex', 'glbert', 'glenarrow']


    if 'ChromeOS' in bot:
      # Just run GLES for now - maybe add gles_msaa4 in the future
      configs = ['gles']

    if 'Chromecast' in bot:
      configs = ['gles']

    # Test coverage counting path renderer.
    if 'CCPR' in bot:
      configs = [c for c in configs if c == 'gl' or c == 'gles']
      args.extend(['--pr', 'ccpr', '--cc', 'true', '--cachePathMasks', 'false'])

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

    if 'Lottie' in bot:
      configs = ['gl']

  tf = api.vars.builder_cfg.get('test_filter')
  if 'All' != tf:
    # Expected format: shard_XX_YY
    parts = tf.split('_')
    if len(parts) == 3:
      args.extend(['--shard', parts[1]])
      args.extend(['--shards', parts[2]])
    else: # pragma: nocover
      raise Exception('Invalid task name - bad shards: %s' % tf)

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image lottie colorImage svg skp'.split(' '))
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
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

  if 'DDL' in bot:
    # The DDL bots just render the large skps and the gms
    remove_from_args('tests')
    remove_from_args('image')
    remove_from_args('colorImage')
    remove_from_args('svg')
  else:
    # Currently, only the DDL bots render skps
    remove_from_args('skp')

  if 'Lottie' in api.vars.builder_cfg.get('extra_config', ''):
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

  if 'Android' in bot or 'iOS' in bot or 'Chromecast' in bot:
    # This test crashes the N9 (perhaps because of large malloc/frees). It also
    # is fairly slow and not platform-specific. So we just disable it on all of
    # Android and iOS. skia:5438
    blacklist('_ test _ GrShape')

  if api.vars.internal_hardware_label == '2':
    # skia:7160
    blacklist('_ test _ SRGBReadWritePixels')
    blacklist('_ test _ SRGBMipMap')
    # skia:9517
    blacklist('_ test _ CharacterizationBackendAllocationTest')
    blacklist('_ test _ ColorTypeBackendAllocationTest')
    blacklist('_ test _ GLBackendAllocationTest')
    blacklist('_ test _ VKBackendAllocationTest')

  if api.vars.internal_hardware_label == '5':
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

  # GM requries canvas->makeSurface() to return a valid surface.
    blacklist([      'pic-8888', 'gm', '_', "blurrect_compare"])
    blacklist(['serialize-8888', 'gm', '_', "blurrect_compare"])

  # GM that not support tiles_rt
  for test in ['complexclip4_bw', 'complexclip4_aa']:
    blacklist([ 'tiles_rt-8888', 'gm', '_', test])

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
  if ('Win8' in bot or 'Win2016' in bot) and 'x86-' in bot:
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

  if 'Chromecast' in bot:
    if 'GPU' in bot:
      # skia:6687
      match.append('~animated-image-blurs')
      match.append('~blur_0.01')
      match.append('~blur_image_filter')
      match.append('~check_small_sigma_offset')
      match.append('~imageblur2')
      match.append('~lighting')
      match.append('~longpathdash')
      match.append('~matrixconvolution')
      match.append('~textblobmixedsizes_df')
      match.append('~textblobrandomfont')
    # Blacklisted to avoid OOM (we see DM just end with "broken pipe")
    match.append('~bigbitmaprect_')
    match.append('~DrawBitmapRect')
    match.append('~drawbitmaprect')
    match.append('~GM_animated-image-blurs')
    match.append('~ImageFilterBlurLargeImage')
    match.append('~savelayer_clipmask')
    match.append('~TextBlobCache')
    match.append('~verylarge')

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

  if 'Vulkan' in bot and api.vars.is_linux and 'IntelIris640' in bot:
    match.extend(['~VkHeapTests']) # skia:6245

  if api.vars.is_linux and 'IntelIris640' in bot:
    match.extend(['~GLPrograms']) # skia:7849

  if 'IntelIris640' in bot or 'IntelHD615' in bot or 'IntelHDGraphics615' in bot:
    match.append('~^SRGBReadWritePixels$') # skia:9225

  if 'Vulkan' in bot and api.vars.is_linux and 'IntelHD405' in bot:
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

  if 'Chorizo' in bot:
    # skia:8869
    blacklist(['_', 'gm', '_', 'compositor_quads_filter'])

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

  if 'LenovoYogaC630' in bot and 'ANGLE' in api.vars.extra_tokens:
    # skia:9275
    blacklist(['_', 'tests', '_', 'GLPrograms'])
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

  return args


def key_params(api):
  """Build a unique key from the builder name (as a list).

  E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
  """
  # Don't bother to include role, which is always Test.
  blacklist = ['role', 'test_filter']

  flat = []
  for k in sorted(api.vars.builder_cfg.keys()):
    if k not in blacklist:
      flat.append(k)
      flat.append(api.vars.builder_cfg[k])

  return flat


def test_steps(api):
  """Run the DM test."""
  b = api.properties['buildername']
  use_hash_file = False
  if upload_dm_results(b):
    host_dm_dir = str(api.flavor.host_dirs.dm_dir)
    api.flavor.create_clean_host_dir(api.path['start_dir'].join('test'))
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

    host_hashes_file = api.vars.tmp_dir.join(hash_filename)
    hashes_file = api.flavor.device_path_join(
        api.flavor.device_dirs.tmp_dir, hash_filename)
    api.run(
        api.python.inline,
        'get uninteresting hashes',
        program="""
        import contextlib
        import math
        import socket
        import sys
        import time
        import urllib2

        HASHES_URL = sys.argv[1]
        RETRIES = 5
        TIMEOUT = 60
        WAIT_BASE = 15

        socket.setdefaulttimeout(TIMEOUT)
        for retry in range(RETRIES):
          try:
            with contextlib.closing(
                urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
              hashes = w.read()
              with open(sys.argv[2], 'w') as f:
                f.write(hashes)
                break
          except Exception as e:
            print 'Failed to get uninteresting hashes from %s:' % HASHES_URL
            print e
            if retry == RETRIES:
              raise
            waittime = WAIT_BASE * math.pow(2, retry)
            print 'Retry in %d seconds.' % waittime
            time.sleep(waittime)
        """,
        args=[api.properties['gold_hashes_url'], host_hashes_file],
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Run DM.
  properties = [
    'gitHash',              api.properties['revision'],
    'builder',              api.vars.builder_name,
    'buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    'task_id',              api.properties['task_id'],
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',         api.vars.issue,
      'patchset',      api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  properties.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  properties.extend(['swarming_task_id', api.vars.swarming_task_id])

  if 'Chromecast' in api.vars.builder_cfg.get('os', ''):
    # Due to limited disk space, we only deal with skps and one image.
    args = [
      'dm',
      '--resourcePath', api.flavor.device_dirs.resource_dir,
      '--skps', api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.resource_dir, 'images', 'color_wheel.jpg'),
      '--nameByHash',
      '--dontReduceOpsTaskSplitting',
      '--properties'
    ] + properties
  else:
    args = [
      'dm',
      '--resourcePath', api.flavor.device_dirs.resource_dir,
      '--skps', api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'dm'),
      '--colorImages', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'colorspace'),
      '--nameByHash',
      '--properties'
    ] + properties

    args.extend(['--svgs', api.flavor.device_dirs.svg_dir])
    if 'Lottie' in api.vars.builder_cfg.get('extra_config', ''):
      args.extend([
        '--lotties',
        api.flavor.device_path_join(
            api.flavor.device_dirs.resource_dir, 'skottie'),
        api.flavor.device_dirs.lotties_dir])

  args.append('--key')
  keys = key_params(api)

  if 'Lottie' in api.vars.builder_cfg.get('extra_config', ''):
    keys.extend(['renderer', 'skottie'])
  if 'DDL' in api.vars.builder_cfg.get('extra_config', ''):
    # 'DDL' style means "--skpViewportSize 2048 --pr ~small"
    keys.extend(['style', 'DDL'])
  else:
    keys.extend(['style', 'default'])

  args.extend(keys)

  if use_hash_file:
    args.extend(['--uninterestingHashesFile', hashes_file])
  if upload_dm_results(b):
    args.extend(['--writePath', api.flavor.device_dirs.dm_dir])

  args.extend(dm_flags(api, api.vars.builder_name))

  # See skia:2789.
  if 'AbandonGpuContext' in api.vars.extra_tokens:
    args.append('--abandonGpuContext')
  if 'PreAbandonGpuContext' in api.vars.extra_tokens:
    args.append('--preAbandonGpuContext')
  if 'ReleaseAndAbandonGpuContext' in api.vars.extra_tokens:
    args.append('--releaseAndAbandonGpuContext')

  api.run(api.flavor.step, 'dm', cmd=args, abort_on_failure=False)

  if upload_dm_results(b):
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  env = {}
  if 'iOS' in api.vars.builder_name:
    env['IOS_BUNDLE_ID'] = 'com.google.dm'
    env['IOS_MOUNT_POINT'] = api.vars.slave_dir.join('mnt_iosdevice')
  with api.context(env=env):
    try:
      if 'Chromecast' in api.vars.builder_name:
        api.flavor.install(resources=True, skps=True)
      elif 'Lottie' in api.vars.builder_name:
        api.flavor.install(resources=True, lotties=True)
      else:
        api.flavor.install(skps=True, images=True, svgs=True, resources=True)
      test_steps(api)
    finally:
      api.flavor.cleanup_steps()
    api.run.check_failure()


TEST_BUILDERS = [
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS6-GPU-MaliT760-arm64-Debug-All-Android',
  ('Test-Android-Clang-GalaxyS6-GPU-MaliT760-arm64-Debug-All'
   '-Android_NoGPUThreads'),
  ('Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All'
   '-Android_Vulkan'),
  'Test-Android-Clang-MotoG4-CPU-Snapdragon617-arm-Release-All-Android',
  'Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-All-Android_CCPR',
  'Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-All-Android',
  'Test-Android-Clang-Nexus7-CPU-Tegra3-arm-Release-All-Android',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android_Vulkan',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm-Debug-All-Android_ASAN',
  'Test-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Debug-All-Android',
  'Test-Android-Clang-Pixel3-GPU-Adreno630-arm64-Debug-All-Android_Vulkan',
  ('Test-ChromeOS-Clang-AcerChromebookR13Convertible-GPU-PowerVRGX6250-'
   'arm-Debug-All'),
  'Test-Chromecast-Clang-Chorizo-CPU-Cortex_A7-arm-Release-All',
  'Test-Chromecast-Clang-Chorizo-GPU-Cortex_A7-arm-Release-All',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-shard_00_10-Coverage',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All'
   '-SK_USE_DISCARDABLE_SCALEDIMAGECACHE'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-Lottie',
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All'
   '-SK_FORCE_RASTER_PIPELINE_BLITTER'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian9-Clang-GCE-GPU-SwiftShader-x86_64-Release-All-SwiftShader',
  'Test-Debian9-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Release-All-Vulkan',
  'Test-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Test-iOS-Clang-iPhone6-GPU-PowerVRGX6450-arm64-Release-All-Metal',
  ('Test-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Release-All'
   '-NativeFonts'),
  'Test-Mac10.13-Clang-MacBookPro11.5-CPU-AVX2-x86_64-Release-All',
  'Test-Mac10.13-Clang-MacBookPro11.5-GPU-RadeonHD8870M-x86_64-Debug-All-Metal',
  ('Test-Mac10.13-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Debug-All'
   '-CommandBuffer'),
  'Test-Mac10.14-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All',
  'Test-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan_Coverage',
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_PreAbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  'Test-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL1',
  'Test-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3',
  'Test-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Lottie',
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-BonusConfigs',
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-NonNVPR',
  ('Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-ReleaseAndAbandonGpuContext'),
  'Test-Win10-Clang-NUC5i7RYH-CPU-AVX2-x86_64-Debug-All-NativeFonts_GDI',
  'Test-Win10-Clang-NUC5i7RYH-GPU-IntelIris6100-x86_64-Release-All-ANGLE',
  'Test-Win10-Clang-NUCD34010WYKH-GPU-IntelHD4400-x86_64-Release-All-ANGLE',
  'Test-Win10-Clang-ShuttleA-GPU-GTX660-x86_64-Release-All-Vulkan',
  'Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Vulkan',
  'Test-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Debug-All-ANGLE',
  'Test-Win10-MSVC-LenovoYogaC630-GPU-Adreno630-arm64-Debug-All-ANGLE',
  'Test-Win2016-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FAAA',
  'Test-Win2016-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FSAA',
  'Test-iOS-Clang-iPadPro-GPU-PowerVRGT7800-arm64-Release-All',
  'Test-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Debug-All-CommandBuffer',
  'Test-Android-Clang-TecnoSpark3Pro-GPU-PowerVRGE8320-arm-Debug-All-Android',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     buildbucket_build_id='123454321',
                     revision='abc123',
                     path_config='kitchen',
                     gold_hashes_url='https://example.com/hashes.txt',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     task_id='task_12345') +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder and not 'LenovoYogaC630' in builder:
      test += api.platform('win', 64)

    if 'Chromecast' in builder:
      test += api.step_data(
          'read chromecast ip',
          stdout=api.raw_io.output('192.168.1.2:5555'))

    yield test

  builder = 'Test-Win8-Clang-Golo-CPU-AVX-x86-Debug-All'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.platform('win', 64) +
    api.properties(patch_storage='gerrit') +
    api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )+
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )

  builder = 'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Debug-All'
  yield (
    api.test('failed_dm') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('symbolized dm', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-All-Android'
  yield (
    api.test('failed_get_hashes') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('get uninteresting hashes', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-All-Android'
  retry_step_name = ('push [START_DIR]/skia/resources/* '
                     '/sdcard/revenge_of_the_skiabot/resources.push '
                     '[START_DIR]/skia/resources/file1')
  yield (
    api.test('failed_push') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('build123-m2--device5')) +
    api.step_data(retry_step_name, retcode=1) +
    api.step_data(retry_step_name + ' (attempt 2)', retcode=1) +
    api.step_data(retry_step_name + ' (attempt 3)', retcode=1)
  )

  retry_step_name = 'adb pull.pull /sdcard/revenge_of_the_skiabot/dm_out'
  yield (
    api.test('failed_pull') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('dm', retcode=1) +
    api.step_data(retry_step_name, retcode=1) +
    api.step_data(retry_step_name + ' (attempt 2)', retcode=1) +
    api.step_data(retry_step_name + ' (attempt 3)', retcode=1)
  )

  yield (
    api.test('internal_bot_2') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   gold_hashes_url='https://example.com/hashes.txt',
                   internal_hardware_label='2',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )

  yield (
    api.test('internal_bot_5') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   gold_hashes_url='https://example.com/hashes.txt',
                   internal_hardware_label='5',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
