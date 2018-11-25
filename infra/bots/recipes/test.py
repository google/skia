# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


DEPS = [
  'core',
  'env',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]


def dm_flags(api, bot):
  args = []
  configs = []
  blacklisted = []

  def blacklist(quad):
    config, src, options, name = quad.split(' ') if type(quad) is str else quad
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

  # 32-bit desktop bots tend to run out of memory, because they have relatively
  # far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
  if '-x86-' in bot and not 'NexusPlayer' in bot:
    args.extend(['--threads', '4'])

  # Nexus7 runs out of memory due to having 4 cores and only 1G RAM.
  if 'CPU' in bot and 'Nexus7' in bot:
    args.extend(['--threads', '2'])

  if 'Chromecast' in bot:
    args.extend(['--threads', '0'])

  # Avoid issues with dynamically exceeding resource cache limits.
  if 'Test' in bot and 'DISCARDABLE' in bot:
    args.extend(['--threads', '0'])

  # See if staying on the main thread helps skia:6748.
  if 'Test-iOS' in bot:
    args.extend(['--threads', '0'])

  # Android's kernel will occasionally attempt to kill our process, using
  # SIGINT, in an effort to free up resources. If requested, that signal
  # is ignored and dm will keep attempting to proceed until we actually
  # exhaust the available resources.
  if ('NexusPlayer' in bot or
      'PixelC' in bot or
      'Chromecast' in bot):
    args.append('--ignoreSigInt')

  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    args.append('--nogpu')

    # These are the canonical configs that we would ideally run on all bots. We
    # may opt out or substitute some below for specific bots
    configs.extend(['8888', 'srgb', 'pdf'])

    # Runs out of memory on Android bots. Everyone else seems fine.
    if 'Android' in bot:
      configs.remove('pdf')

    if '-GCE-' in bot:
      configs.extend(['g8'])
      configs.extend(['565'])
      configs.extend(['f16'])
      configs.extend(['lite-8888'])              # Experimental display list.
      configs.extend(['gbr-8888'])

    configs.extend(mode + '-8888' for mode in ['serialize', 'tiles_rt', 'pic'])

    # This bot only differs from vanilla CPU bots in 8888 config.
    if 'SK_FORCE_RASTER_PIPELINE_BLITTER' in bot:
      configs = ['8888', 'srgb']

    if 'FSAA' in bot or 'FAAA' in bot or 'FDAA' in bot:
      # Scan converters shouldn't really be sensitive to different color
      # configurations.
      configs = ['8888', 'tiles_rt-8888']

    if 'NativeFonts' in bot:
      configs = ['8888']

    # Just do the basic config on Chromecast to avoid OOM.
    if 'Chromecast' in bot:
      configs = ['8888']

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
      sample_count = ''
    elif 'ChromeOS' in bot:
      gl_prefix = 'gles'

    configs.extend([gl_prefix, gl_prefix + 'dft', gl_prefix + 'srgb'])
    if sample_count is not '':
      configs.append(gl_prefix + 'msaa' + sample_count)

    # The NP produces a long error stream when we run with MSAA. The Tegra3 just
    # doesn't support it.
    if ('NexusPlayer' in bot or
        'Tegra3'      in bot or
        # We aren't interested in fixing msaa bugs on current iOS devices.
        'iPad4' in bot or
        'iPadPro' in bot or
        'iPhone6' in bot or
        'iPhone7' in bot or
        # skia:5792
        'IntelHD530'   in bot or
        'IntelIris540' in bot):
      configs = [x for x in configs if 'msaa' not in x]

    # The NP produces different images for dft on every run.
    if 'NexusPlayer' in bot:
      configs = [x for x in configs if 'dft' not in x]

    if '-TSAN' not in bot and sample_count is not '':
      if ('TegraK1'    in bot or
          'TegraX1'    in bot or
          'GTX550Ti'   in bot or
          'GTX660'     in bot or
          'QuadroP400' in bot or
          ('GT610' in bot and 'Ubuntu17' not in bot)):
        configs.append(gl_prefix + 'nvprdit' + sample_count)

    # We want to test both the OpenGL config and the GLES config on Linux Intel:
    # GL is used by Chrome, GLES is used by ChromeOS.
    # Also do the Ganesh threading verification test (render with and without
    # worker threads, using only the SW path renderer, and compare the results).
    if 'Intel' in bot and api.vars.is_linux:
      configs.extend(['gles', 'glesdft', 'glessrgb', 'gltestthreading'])
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

    # The following devices do not support glessrgb.
    if 'glessrgb' in configs:
      if ('IntelHD405'    in bot or
          'IntelIris640'  in bot or
          'IntelBayTrail' in bot or
          'IntelHD2000'   in bot or
          'AndroidOne'    in bot or
          'Nexus7'        in bot or
          'NexusPlayer'   in bot):
        configs.remove('glessrgb')

    # Test SkColorSpaceXformCanvas on a few bots
    if 'GTX1070' in bot:
      configs.append('gbr-gl')

    # CommandBuffer bot *only* runs the command_buffer config.
    if 'CommandBuffer' in bot:
      configs = ['commandbuffer']

    # ANGLE bot *only* runs the angle configs
    if 'ANGLE' in bot:
      configs = ['angle_d3d11_es2',
                 'angle_d3d9_es2',
                 'angle_gl_es2',
                 'angle_d3d11_es3']
      if sample_count is not '':
        configs.append('angle_d3d11_es2_msaa' + sample_count)
        configs.append('angle_d3d11_es3_msaa' + sample_count)

    # Vulkan bot *only* runs the vk config.
    if 'Vulkan' in bot:
      configs = ['vk']

    if 'ChromeOS' in bot:
      # Just run GLES for now - maybe add gles_msaa4 in the future
      configs = ['gles']

    if 'Chromecast' in bot:
      configs = ['gles']

    # Test coverage counting path renderer.
    if 'CCPR' in bot:
      configs = [c for c in configs if c == 'gl' or c == 'gles']
      args.extend(['--pr', 'ccpr', '--cachePathMasks', 'false'])

  tf = api.vars.builder_cfg.get('test_filter')
  if 'All' != tf:
    # Expected format: shard_XX_YY
    parts = tf.split('_')
    if len(parts) == 3:
      args.extend(['--shard', parts[1]])
      args.extend(['--shards', parts[2]])
    else:
      raise Exception('Invalid task name - bad shards') #pragma: nocover

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image colorImage svg'.split(' '))
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

  # TODO: ???
  blacklist('f16 _ _ dstreadshuffle')
  blacklist('glsrgb image _ _')
  blacklist('glessrgb image _ _')

  # Not any point to running these.
  blacklist('gbr-8888 image _ _')
  blacklist('gbr-8888 colorImage _ _')

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

  if 'Android' in bot or 'iOS' in bot or 'Chromecast' in bot:
    # This test crashes the N9 (perhaps because of large malloc/frees). It also
    # is fairly slow and not platform-specific. So we just disable it on all of
    # Android and iOS. skia:5438
    blacklist('_ test _ GrShape')

  if api.vars.internal_hardware_label == 1:
    # skia:7046
    blacklist('_ test _ WritePixelsNonTexture_Gpu')
    blacklist('_ test _ WritePixels_Gpu')
    blacklist('_ test _ GrSurfaceRenderability')
    blacklist('_ test _ ES2BlendWithNoTexture')

  if api.vars.internal_hardware_label == 2:
    # skia:7160
    blacklist('_ test _ SRGBReadWritePixels')
    blacklist('_ test _ SRGBMipMap')


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
                       'imagemasksubset']

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

  # Not expected to round trip encoding/decoding.
  bad_serialize_gms.append('all_bitmap_configs')
  bad_serialize_gms.append('makecolorspace')

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
    blacklist([  'lite-8888', 'gm', '_', test])
  # skia:4703
  for test in ['image-cacherator-from-picture',
               'image-cacherator-from-raster',
               'image-cacherator-from-ctable']:
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

  # GM that requires raster-backed canvas
  for test in ['gamut', 'complexclip4_bw', 'complexclip4_aa']:
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist([     'lite-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

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

  if 'IntelHD405' in bot and 'Ubuntu16' in bot:
    # skia:6331
    blacklist(['glmsaa8',   'image', 'gen_codec_gpu', 'abnormal.wbmp'])
    blacklist(['glesmsaa4', 'image', 'gen_codec_gpu', 'abnormal.wbmp'])

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
    if sample_count is not '':
      gl_msaa_config = gl_prefix + 'msaa' + sample_count
      blacklist([gl_msaa_config, 'gm', '_', 'imageblurtiled'])
      blacklist([gl_msaa_config, 'gm', '_', 'imagefiltersbase'])

  match = []
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')

  if 'Valgrind' in bot and 'PreAbandonGpuContext' in bot:
    # skia:6575
    match.append('~multipicturedraw_')

  if 'CommandBuffer' in bot:
    # https://crbug.com/697030
    match.append('~HalfFloatAlphaTextureTest')

  if 'AndroidOne' in bot:  # skia:4711
    match.append('~WritePixels')

  if 'Chromecast' in bot:
    if 'GPU' in bot:
      # skia:6687
      match.append('~matrixconvolution')
      match.append('~blur_image_filter')
      match.append('~blur_0.01')
      match.append('~lighting')
      match.append('~imageblur2')
      match.append('~animated-image-blurs')
    # skia:7497
    match.append('~readpixels') # dies with "Caught signal 7 [Bus error]"
    match.append('~SkRasterPipeline_tail')
    # Blacklisted to avoid OOM (we see DM just end with "broken pipe")
    match.append('~GM_animated-image-blurs')
    match.append('~verylarge')
    match.append('~ImageFilterBlurLargeImage')
    match.append('~TextBlobCache')
    match.append('~bigbitmaprect_')
    match.append('~savelayer_clipmask')
    match.append('~DrawBitmapRect')

  if 'GalaxyS6' in bot:
    match.append('~SpecialImage') # skia:6338
    match.append('~skbug6653') # skia:6653

  if 'GalaxyS7_G930A' in bot:
    match.append('~WritePixels') # skia:6427

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

  if 'Vulkan' in bot and 'NexusPlayer' in bot:
    # skia:6132
    match.append('~^tilemodes$')
    match.append('~tilemodes_npot$')
    match.append('~scaled_tilemodes$')
    match.append('~emboss')
    match.append('~^bitmapfilters$')
    match.append('~^shadertext$')
    match.append('~^FullScreenClearWithLayers$') #skia:7191
    match.append('~^GrDefaultPathRendererTest$') #skia:7244
    match.append('~^GrMSAAPathRendererTest$') #skia:7244
    # skia:7018
    match.extend(['~^ClearOp$',
                  '~^ComposedImageFilterBounds_Gpu$',
                  '~^ImageEncode_Gpu$',
                  '~^ImageFilterFailAffectsTransparentBlack_Gpu$',
                  '~^ImageFilterZeroBlurSigma_Gpu$',
                  '~^ImageNewShader_GPU$',
                  '~^ImageReadPixels_Gpu$',
                  '~^ImageScalePixels_Gpu$',
                  '~^ReadWriteAlpha$',
                  '~^SpecialImage_DeferredGpu$',
                  '~^SpecialImage_Gpu$',
                  '~^SurfaceSemaphores$'])

  if 'Vulkan' in bot and api.vars.is_linux and 'IntelIris640' in bot:
    match.extend(['~VkHeapTests']) # skia:6245

  if 'Vulkan' in bot and api.vars.is_linux and 'IntelHD405' in bot:
    # skia:7322
    blacklist(['vk', 'gm', '_', 'skbug_257'])
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
    match.append('~^WritePixels_Gpu$')
    match.append('~^skbug6653$')

  if 'Vulkan' in bot and 'IntelIris540' in bot and 'Win' in bot:
    # skia:6398
    blacklist(['vk', 'gm', '_', 'aarectmodes'])
    blacklist(['vk', 'gm', '_', 'aaxfermodes'])
    blacklist(['vk', 'gm', '_', 'arithmode'])
    blacklist(['vk', 'gm', '_', 'composeshader'])
    blacklist(['vk', 'gm', '_', 'composeshader_alpha'])
    blacklist(['vk', 'gm', '_', 'composeshader_bitmap'])
    blacklist(['vk', 'gm', '_', 'composeshader_bitmap2'])
    blacklist(['vk', 'gm', '_', 'dont_clip_to_layer'])
    blacklist(['vk', 'gm', '_', 'dftext'])
    blacklist(['vk', 'gm', '_', 'drawregionmodes'])
    blacklist(['vk', 'gm', '_', 'filterfastbounds'])
    blacklist(['vk', 'gm', '_', 'fontcache'])
    blacklist(['vk', 'gm', '_', 'fontmgr_iter'])
    blacklist(['vk', 'gm', '_', 'fontmgr_iter_factory'])
    blacklist(['vk', 'gm', '_', 'fontmgr_match'])
    blacklist(['vk', 'gm', '_', 'fontscaler'])
    blacklist(['vk', 'gm', '_', 'fontscalerdistortable'])
    blacklist(['vk', 'gm', '_', 'gammagradienttext'])
    blacklist(['vk', 'gm', '_', 'gammatext'])
    blacklist(['vk', 'gm', '_', 'gradtext'])
    blacklist(['vk', 'gm', '_', 'hairmodes'])
    blacklist(['vk', 'gm', '_', 'imagefilters_xfermodes'])
    blacklist(['vk', 'gm', '_', 'imagefiltersclipped'])
    blacklist(['vk', 'gm', '_', 'imagefiltersgraph'])
    blacklist(['vk', 'gm', '_', 'imagefiltersscaled'])
    blacklist(['vk', 'gm', '_', 'imagefiltersstroked'])
    blacklist(['vk', 'gm', '_', 'imagefilterstransformed'])
    blacklist(['vk', 'gm', '_', 'imageresizetiled'])
    blacklist(['vk', 'gm', '_', 'lcdblendmodes'])
    blacklist(['vk', 'gm', '_', 'lcdoverlap'])
    blacklist(['vk', 'gm', '_', 'lcdtext'])
    blacklist(['vk', 'gm', '_', 'lcdtextsize'])
    blacklist(['vk', 'gm', '_', 'matriximagefilter'])
    blacklist(['vk', 'gm', '_', 'mixedtextblobs'])
    blacklist(['vk', 'gm', '_', 'mixershader'])
    blacklist(['vk', 'gm', '_', 'pictureimagefilter'])
    blacklist(['vk', 'gm', '_', 'resizeimagefilter'])
    blacklist(['vk', 'gm', '_', 'rotate_imagefilter'])
    blacklist(['vk', 'gm', '_', 'savelayer_lcdtext'])
    blacklist(['vk', 'gm', '_', 'srcmode'])
    blacklist(['vk', 'gm', '_', 'surfaceprops'])
    blacklist(['vk', 'gm', '_', 'textblobgeometrychange'])
    blacklist(['vk', 'gm', '_', 'textbloblooper'])
    blacklist(['vk', 'gm', '_', 'textblobmixedsizes'])
    blacklist(['vk', 'gm', '_', 'textblobmixedsizes_df'])
    blacklist(['vk', 'gm', '_', 'textblobrandomfont'])
    blacklist(['vk', 'gm', '_', 'textfilter_color'])
    blacklist(['vk', 'gm', '_', 'textfilter_image'])
    blacklist(['vk', 'gm', '_', 'typefacerendering'])
    blacklist(['vk', 'gm', '_', 'varied_text_clipped_lcd'])
    blacklist(['vk', 'gm', '_', 'varied_text_ignorable_clip_lcd'])
    blacklist(['vk', 'gm', '_', 'xfermodeimagefilter'])
    match.append('~ApplyGamma')
    match.append('~ComposedImageFilterBounds_Gpu')
    match.append('~DeferredTextureImage')
    match.append('~GrMeshTest')
    match.append('~ImageFilterFailAffectsTransparentBlack_Gpu')
    match.append('~ImageFilterZeroBlurSigma_Gpu')
    match.append('~ImageNewShader_GPU')
    match.append('~NewTextureFromPixmap')
    match.append('~ReadPixels_Gpu')
    match.append('~ReadPixels_Texture')
    match.append('~ReadWriteAlpha')
    match.append('~skbug6653')
    match.append('~SRGBReadWritePixels')
    match.append('~SpecialImage_DeferredGpu')
    match.append('~SpecialImage_Gpu')
    match.append('~WritePixels_Gpu')
    match.append('~WritePixelsNonTexture_Gpu')
    match.append('~XfermodeImageFilterCroppedInput_Gpu')
    match.append('~GrDefaultPathRendererTest') #skia:7244
    match.append('~GrMSAAPathRendererTest') #skia:7244
    match.append('~SkImage_makeNonTextureImage')

  if (('RadeonR9M470X' in bot or 'RadeonHD7770' in bot) and 'ANGLE' in bot):
    # skia:7096
    match.append('~PinnedImageTest')

  if 'IntelIris540' in bot and 'ANGLE' in bot:
    for config in ['angle_d3d9_es2', 'angle_d3d11_es2', 'angle_gl_es2']:
      # skia:6103
      blacklist([config, 'gm', '_', 'multipicturedraw_invpathclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_noclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_pathclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_rectclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_rrectclip_simple'])
      # skia:6141
      blacklist([config, 'gm', '_', 'discard'])

  if ('IntelIris6100' in bot or 'IntelHD4400' in bot) and 'ANGLE' in bot:
    # skia:6857
    blacklist(['angle_d3d9_es2', 'gm', '_', 'lighting'])

  if 'PowerVRGX6250' in bot:
    match.append('~gradients_view_perspective_nodither') #skia:6972

  if '-arm-' in bot and 'ASAN' in bot:
    # TODO: can we run with env allocator_may_return_null=1 instead?
    match.append('~BadImage')

  if blacklisted:
    args.append('--blacklist')
    args.extend(blacklisted)

  if match:
    args.append('--match')
    args.extend(match)

  # These bots run out of memory running RAW codec tests. Do not run them in
  # parallel
  if 'NexusPlayer' in bot or 'Nexus5' in bot or 'Nexus9' in bot:
    args.append('--noRAW_threading')

  if 'FSAA' in bot:
    args.extend(['--analyticAA', 'false', '--deltaAA', 'false'])
  if 'FAAA' in bot:
    args.extend(['--deltaAA', 'false', '--forceAnalyticAA'])
  if 'FDAA' in bot:
    args.extend(['--deltaAA', '--forceDeltaAA'])

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
  use_hash_file = False
  if api.vars.upload_dm_results:
    host_dm_dir = str(api.vars.dm_dir)
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

    # Ensure that the tmp_dir exists.
    api.run.run_once(api.file.ensure_directory,
                     'makedirs tmp_dir',
                     api.vars.tmp_dir)

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

        HASHES_URL = 'https://storage.googleapis.com/skia-infra-gm/hash_files/gold-prod-hashes.txt'
        RETRIES = 5
        TIMEOUT = 60
        WAIT_BASE = 15

        socket.setdefaulttimeout(TIMEOUT)
        for retry in range(RETRIES):
          try:
            with contextlib.closing(
                urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
              hashes = w.read()
              with open(sys.argv[1], 'w') as f:
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
        args=[host_hashes_file],
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Run DM.
  properties = [
    'gitHash',              api.vars.got_revision,
    'builder',              api.vars.builder_name,
    'buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
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

  args.append('--key')
  args.extend(key_params(api))

  if use_hash_file:
    args.extend(['--uninterestingHashesFile', hashes_file])
  if api.vars.upload_dm_results:
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

  if api.vars.upload_dm_results:
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.vars.dm_dir)


def RunSteps(api):
  api.core.setup()
  env = {}
  if 'iOS' in api.vars.builder_name:
    env['IOS_BUNDLE_ID'] = 'com.google.dm'
    env['IOS_MOUNT_POINT'] = api.vars.slave_dir.join('mnt_iosdevice')
  with api.context(env=env):
    try:
      if 'Chromecast' in api.vars.builder_name:
        api.flavor.install(resources=True, skps=True)
      else:
        api.flavor.install_everything()
      test_steps(api)
    finally:
      api.flavor.cleanup_steps()
    api.run.check_failure()


TEST_BUILDERS = [
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS6-GPU-MaliT760-arm64-Debug-All-Android',
  'Test-Android-Clang-GalaxyS7_G930A-GPU-Adreno530-arm64-Debug-All-Android',
  'Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-All-Android',
  'Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-All-Android_CCPR',
  'Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm-Debug-All-Android_ASAN',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android_ASAN',
  ('Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All'
   '-Android_NoGPUThreads'),
  'Test-Android-Clang-Nexus7-CPU-Tegra3-arm-Release-All-Android',
  'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-All-Android',
  'Test-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Release-All-Android',
  'Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-All-Android_Vulkan',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android_CCPR',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android_Vulkan',
  'Test-ChromeOS-Clang-ASUSChromebookFlipC100-GPU-MaliT764-arm-Debug-All',
  ('Test-ChromeOS-Clang-AcerChromebookR13Convertible-GPU-PowerVRGX6250-'
   'arm-Debug-All'),
  'Test-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Release-All',
  'Test-Chromecast-GCC-Chorizo-GPU-Cortex_A7-arm-Release-All',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-shard_00_10-Coverage',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All'
   '-SK_USE_DISCARDABLE_SCALEDIMAGECACHE'),
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All'
   '-SK_FORCE_RASTER_PIPELINE_BLITTER'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian9-GCC-GCE-CPU-AVX2-x86-Debug-All',
  'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Debug-All',
  'Test-Mac-Clang-MacMini7.1-CPU-AVX-x86_64-Release-All',
  'Test-Mac-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Debug-All-CommandBuffer',
  'Test-Ubuntu16-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Debug-All',
  'Test-Ubuntu16-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Release-All-Vulkan',
  'Test-Ubuntu16-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Test-Ubuntu16-Clang-NUCDE3815TYKHE-GPU-IntelBayTrail-x86_64-Debug-All',
  'Test-Ubuntu17-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan_Coverage',
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_PreAbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_SK_CPU_LIMIT_SSE41'),
  'Test-Win10-Clang-AlphaR2-GPU-RadeonR9M470X-x86_64-Debug-All-ANGLE',
  'Test-Win10-Clang-AlphaR2-GPU-RadeonR9M470X-x86_64-Debug-All-Vulkan',
  ('Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-ReleaseAndAbandonGpuContext'),
  'Test-Win10-Clang-NUC5i7RYH-CPU-AVX2-x86_64-Debug-All-NativeFonts',
  'Test-Win10-Clang-NUC5i7RYH-CPU-AVX2-x86_64-Debug-All-NativeFonts_GDI',
  'Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All-ANGLE',
  'Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All-Vulkan',
  'Test-Win10-Clang-NUCD34010WYKH-GPU-IntelHD4400-x86_64-Release-All-ANGLE',
  'Test-Win10-Clang-ShuttleA-GPU-GTX660-x86_64-Debug-All-Vulkan',
  'Test-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Debug-All-ANGLE',
  'Test-Win10-Clang-ZBOX-GPU-GTX1070-x86_64-Debug-All',
  'Test-Win10-Clang-ZBOX-GPU-GTX1070-x86_64-Debug-All-Vulkan',
  'Test-Win2016-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FAAA',
  'Test-Win2016-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FDAA',
  'Test-Win2016-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FSAA',
  'Test-Win8-MSVC-Golo-CPU-AVX-x86-Debug-All',
  'Test-iOS-Clang-iPadPro-GPU-GT7800-arm64-Release-All',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     buildbucket_build_id='123454321',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
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
    if 'Win' in builder:
      test += api.platform('win', 64)

    if 'Chromecast' in builder:
      test += api.step_data(
          'read chromecast ip',
          stdout=api.raw_io.output('192.168.1.2:5555'))

    if 'ChromeOS' in builder:
      test += api.step_data(
          'read chromeos ip',
          stdout=api.raw_io.output('{"user_ip":"foo@127.0.0.1"}'))

    yield test

  builder = 'Test-Win2k8-MSVC-GCE-CPU-AVX2-x86_64-Release-All'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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

  builder = ('Test-Android-Clang-NexusPlayer-CPU-Moorefield-x86-'
             'Debug-All-Android')
  yield (
    api.test('failed_push') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('push [START_DIR]/skia/resources/* '+
                  '/sdcard/revenge_of_the_skiabot/resources', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-All-Android'
  yield (
    api.test('failed_pull') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('pull /sdcard/revenge_of_the_skiabot/dm_out '+
                  '[CUSTOM_[SWARM_OUT_DIR]]/dm', retcode=1)
  )

  yield (
    api.test('internal_bot_1') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   internal_hardware_label=1) +
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
    api.test('internal_bot_2') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   internal_hardware_label=2) +
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
