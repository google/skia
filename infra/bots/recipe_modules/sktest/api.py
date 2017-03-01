# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


from recipe_engine import recipe_api


def dm_flags(bot):
  args = []

  # 32-bit desktop bots tend to run out of memory, because they have relatively
  # far more cores than RAM (e.g. 32 cores, 3G RAM).  Hold them back a bit.
  if '-x86-' in bot and not 'NexusPlayer' in bot:
    args.extend('--threads 4'.split(' '))

  # These are the canonical configs that we would ideally run on all bots. We
  # may opt out or substitute some below for specific bots
  configs = ['8888', 'srgb', 'gpu', 'gpudft', 'gpusrgb', 'pdf']
  # Add in either msaa4 or msaa16 to the canonical set of configs to run
  if 'Android' in bot or 'iOS' in bot:
    configs.append('msaa4')
  else:
    configs.append('msaa16')

  # The NP produces a long error stream when we run with MSAA. The Tegra3 just
  # doesn't support it.
  if ('NexusPlayer' in bot or
      'Tegra3'      in bot or
      # We aren't interested in fixing msaa bugs on iPad4.
      'iPad4' in bot or
      # skia:5792
      'iHD530'       in bot or
      'IntelIris540' in bot):
    configs = [x for x in configs if 'msaa' not in x]

  # The NP produces different images for dft on every run.
  if 'NexusPlayer' in bot:
    configs = [x for x in configs if 'gpudft' not in x]

  # Runs out of memory on Android bots.  Everyone else seems fine.
  if 'Android' in bot:
    configs.remove('pdf')

  if '-GCE-' in bot:
    configs.extend(['565'])
    configs.extend(['f16'])
    configs.extend(['sp-8888', '2ndpic-8888'])   # Test niche uses of SkPicture.
    configs.extend(['lite-8888'])                # Experimental display list.

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

  # We want to test the OpenGL config not the GLES config on the Shield
  if 'NVIDIA_Shield' in bot:
    configs = [x.replace('gpu', 'gl') for x in configs]
    configs = [x.replace('msaa', 'glmsaa') for x in configs]
    configs = [x.replace('nvpr', 'glnvpr') for x in configs]

  # NP is running out of RAM when we run all these modes.  skia:3255
  if 'NexusPlayer' not in bot:
    configs.extend(mode + '-8888' for mode in
                   ['serialize', 'tiles_rt', 'pic'])

  # Test instanced rendering on a limited number of platforms
  if 'Nexus6' in bot:
    configs.append('esinst') # esinst4 isn't working yet on Adreno.
  elif 'NVIDIA_Shield' in bot:
    # Multisampled instanced configs use nvpr.
    configs = [x.replace('glnvpr', 'glinst') for x in configs]
    configs.append('glinst')
  elif 'PixelC' in bot:
    # Multisampled instanced configs use nvpr.
    configs = [x.replace('nvpr', 'esinst') for x in configs]
    configs.append('esinst')
  elif 'MacMini6.2' in bot:
    configs.extend(['glinst', 'glinst16'])

  # CommandBuffer bot *only* runs the command_buffer config.
  if 'CommandBuffer' in bot:
    configs = ['commandbuffer']

  # ANGLE bot *only* runs the angle configs
  if 'ANGLE' in bot:
    configs = ['angle_d3d11_es2',
               'angle_d3d9_es2',
               'angle_d3d11_es2_msaa4',
               'angle_gl_es2']

  # Vulkan bot *only* runs the vk config.
  if 'Vulkan' in bot:
    configs = ['vk']

  args.append('--config')
  args.extend(configs)

  # Run tests, gms, and image decoding tests everywhere.
  args.extend('--src tests gm image colorImage'.split(' '))
  if 'Vulkan' not in bot or 'NexusPlayer' not in bot:
    args.append('svg')

  blacklisted = []
  def blacklist(quad):
    config, src, options, name = quad.split(' ') if type(quad) is str else quad
    if config == '_' or config in configs:
      blacklisted.extend([config, src, options, name])

  # TODO: ???
  blacklist('f16 _ _ dstreadshuffle')
  blacklist('gpusrgb image _ _')
  blacklist('glsrgb image _ _')

  # Decoder tests are now performing gamma correct decodes.  This means
  # that, when viewing the results, we need to perform a gamma correct
  # encode to PNG.  Therefore, we run the image tests in srgb mode instead
  # of 8888.
  blacklist('8888 image _ _')

  if 'Valgrind' in bot:
    # These take 18+ hours to run.
    blacklist('pdf gm _ fontmgr_iter')
    blacklist('pdf _ _ PANO_20121023_214540.jpg')
    blacklist('pdf skp _ worldjournal')
    blacklist('pdf skp _ desk_baidu.skp')
    blacklist('pdf skp _ desk_wikipedia.skp')
    blacklist('_ svg _ _')

  if 'iOS' in bot:
    blacklist('gpu skp _ _')
    blacklist('msaa skp _ _')
    blacklist('msaa16 gm _ tilemodesProcess')

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

  # WIC fails on questionable bmps and arithmetic jpegs
  if 'Win' in bot:
    blacklist('_ image gen_platf rle8-height-negative.bmp')
    blacklist('_ image gen_platf rle4-height-negative.bmp')
    blacklist('_ image gen_platf pal8os2v2.bmp')
    blacklist('_ image gen_platf pal8os2v2-16.bmp')
    blacklist('_ image gen_platf rgba32abf.bmp')
    blacklist('_ image gen_platf rgb24prof.bmp')
    blacklist('_ image gen_platf rgb24lprof.bmp')
    blacklist('_ image gen_platf 8bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 4bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 32bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf 24bpp-pixeldata-cropped.bmp')
    blacklist('_ image gen_platf testimgari.jpg')
    if 'x86_64' in bot and 'CPU' in bot:
      # This GM triggers a SkSmallAllocator assert.
      blacklist('_ gm _ composeshader_bitmap')

  if 'Android' in bot or 'iOS' in bot:
    # This test crashes the N9 (perhaps because of large malloc/frees). It also
    # is fairly slow and not platform-specific. So we just disable it on all of
    # Android and iOS. skia:5438
    blacklist('_ test _ GrShape')

  if 'Win8' in bot:
    # bungeman: "Doesn't work on Windows anyway, produces unstable GMs with
    # 'Unexpected error' from DirectWrite"
    blacklist('_ gm _ fontscalerdistortable')
    # skia:5636
    blacklist('_ svg _ Nebraska-StateSeal.svg')

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

  for test in bad_serialize_gms:
    blacklist(['serialize-8888', 'gm', '_', test])

  if 'Mac' not in bot:
    for test in ['bleed_alpha_image', 'bleed_alpha_image_shader']:
      blacklist(['serialize-8888', 'gm', '_', test])
  # It looks like we skip these only for out-of-memory concerns.
  if 'Win' in bot or 'Android' in bot:
    for test in ['verylargebitmap', 'verylarge_picture_image']:
      blacklist(['serialize-8888', 'gm', '_', test])

  # skia:4769
  for test in ['drawfilter']:
    blacklist([    'sp-8888', 'gm', '_', test])
    blacklist([   'pic-8888', 'gm', '_', test])
    blacklist(['2ndpic-8888', 'gm', '_', test])
    blacklist([  'lite-8888', 'gm', '_', test])
  # skia:4703
  for test in ['image-cacherator-from-picture',
               'image-cacherator-from-raster',
               'image-cacherator-from-ctable']:
    blacklist([       'sp-8888', 'gm', '_', test])
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist([   '2ndpic-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

  # GM that requires raster-backed canvas
  for test in ['gamut', 'complexclip4_bw', 'complexclip4_aa']:
    blacklist([       'sp-8888', 'gm', '_', test])
    blacklist([      'pic-8888', 'gm', '_', test])
    blacklist([     'lite-8888', 'gm', '_', test])
    blacklist([   '2ndpic-8888', 'gm', '_', test])
    blacklist(['serialize-8888', 'gm', '_', test])

  # GM that not support tiles_rt
  for test in ['complexclip4_bw', 'complexclip4_aa']:
    blacklist([ 'tiles_rt-8888', 'gm', '_', test])

  # Extensions for RAW images
  r = ["arw", "cr2", "dng", "nef", "nrw", "orf", "raf", "rw2", "pef", "srw",
       "ARW", "CR2", "DNG", "NEF", "NRW", "ORF", "RAF", "RW2", "PEF", "SRW"]

  # skbug.com/4888
  # Blacklist RAW images (and a few large PNGs) on GPU bots
  # until we can resolve failures.
  # Also blacklisted on 32-bit Win2k8 for F16 OOM errors.
  if 'GPU' in bot or ('Win2k8' in bot and 'x86-' in bot):
    blacklist('_ image _ interlaced1.png')
    blacklist('_ image _ interlaced2.png')
    blacklist('_ image _ interlaced3.png')
    for raw_ext in r:
      blacklist('_ image _ .%s' % raw_ext)

  # Large image that overwhelms older Mac bots
  if 'MacMini4.1-GPU' in bot:
    blacklist('_ image _ abnormal.wbmp')
    blacklist(['msaa16', 'gm', '_', 'blurcircles'])

  if 'Nexus5' in bot:
    # skia:5876
    blacklist(['_', 'gm', '_', 'encode-platform'])

  if 'AndroidOne-GPU' in bot:  # skia:4697, skia:4704, skia:4694, skia:4705
    blacklist(['_',     'gm', '_', 'bigblurs'])
    blacklist(['_',     'gm', '_', 'bleed'])
    blacklist(['_',     'gm', '_', 'bleed_alpha_bmp'])
    blacklist(['_',     'gm', '_', 'bleed_alpha_bmp_shader'])
    blacklist(['_',     'gm', '_', 'bleed_alpha_image'])
    blacklist(['_',     'gm', '_', 'bleed_alpha_image_shader'])
    blacklist(['_',     'gm', '_', 'bleed_image'])
    blacklist(['_',     'gm', '_', 'dropshadowimagefilter'])
    blacklist(['_',     'gm', '_', 'filterfastbounds'])
    blacklist(['gpu',   'gm', '_', 'imageblurtiled'])
    blacklist(['msaa4', 'gm', '_', 'imageblurtiled'])
    blacklist(['msaa4', 'gm', '_', 'imagefiltersbase'])
    blacklist(['_',     'gm', '_', 'imagefiltersclipped'])
    blacklist(['_',     'gm', '_', 'imagefiltersscaled'])
    blacklist(['_',     'gm', '_', 'imageresizetiled'])
    blacklist(['_',     'gm', '_', 'matrixconvolution'])
    blacklist(['_',     'gm', '_', 'strokedlines'])

  match = []
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')

  if 'AndroidOne' in bot:  # skia:4711
    match.append('~WritePixels')

  if 'NexusPlayer' in bot:
    match.append('~ResourceCache')

  if 'Nexus10' in bot:
    match.append('~CopySurface') # skia:5509
    match.append('~SRGBReadWritePixels') # skia:6097

  if 'GalaxyJ5' in bot:
    match.append('~SRGBReadWritePixels') # skia:6097

  if 'ANGLE' in bot and 'Debug' in bot:
    match.append('~GLPrograms') # skia:4717

  if 'MSAN' in bot:
    match.extend(['~Once', '~Shared'])  # Not sure what's up with these tests.

  if 'TSAN' in bot:
    match.extend(['~ReadWriteAlpha'])   # Flaky on TSAN-covered on nvidia bots.
    match.extend(['~RGBA4444TextureTest',  # Flakier than they are important.
                  '~RGB565TextureTest'])

  if 'Vulkan' in bot and 'Adreno' in bot:
    # skia:5777
    match.extend(['~XfermodeImageFilterCroppedInput',
                  '~GrTextureStripAtlasFlush',
                  '~CopySurface'])

  if 'Vulkan' in bot and 'NexusPlayer' in bot:
    match.extend(['~hardstop_gradient', # skia:6037
                  '~gradients_dup_color_stops',  # skia:6037
                  '~gradients_no_texture$', # skia:6132
                  '~tilemodes', # skia:6132
                  '~shadertext$', # skia:6132
                  '~bitmapfilters', # skia:6132
                  '~GrContextFactory_abandon']) #skia:6209

  if 'Vulkan' in bot and 'GTX1070' in bot and 'Win' in bot:
    # skia:6092
    match.append('~GPUMemorySize')

  if 'Vulkan' in bot and 'IntelIris540' in bot and 'Ubuntu' in bot:
    match.extend(['~VkHeapTests', # skia:6245
                  '~XfermodeImageFilterCroppedInput_Gpu']) #skia:6280

  if 'IntelIris540' in bot and 'ANGLE' in bot:
    match.append('~IntTexture') # skia:6086
    blacklist(['_', 'gm', '_', 'discard']) # skia:6141
    # skia:6103
    for config in ['angle_d3d9_es2', 'angle_d3d11_es2', 'angle_gl_es2']:
      blacklist([config, 'gm', '_', 'multipicturedraw_invpathclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_noclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_pathclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_rectclip_simple'])
      blacklist([config, 'gm', '_', 'multipicturedraw_rrectclip_simple'])

  if 'Vivante' in bot:
    # This causes the bot to spin for >3.5 hours.
    blacklist(['_', 'gm', '_', 'scaled_tilemodes_npot'])

  if blacklisted:
    args.append('--blacklist')
    args.extend(blacklisted)

  if match:
    args.append('--match')
    args.extend(match)

  # These bots run out of memory running RAW codec tests. Do not run them in
  # parallel
  if ('NexusPlayer' in bot or 'Nexus5' in bot or 'Nexus9' in bot
      or 'Win8-MSVC-ShuttleB' in bot):
    args.append('--noRAW_threading')

  return args


def key_params(api):
  """Build a unique key from the builder name (as a list).

  E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
  """
  # Don't bother to include role, which is always Test.
  # TryBots are uploaded elsewhere so they can use the same key.
  blacklist = ['role', 'is_trybot']

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
    # This must run before we write anything into
    # api.flavor.device_dirs.dm_dir or we may end up deleting our
    # output on machines where they're the same.
    api.flavor.create_clean_host_dir(api.vars.dm_dir)
    host_dm_dir = str(api.vars.dm_dir)
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

    # Ensure that the tmp_dir exists.
    api.run.run_once(api.file.makedirs,
                           'tmp_dir',
                           api.vars.tmp_dir,
                           infra_step=True)

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

        HASHES_URL = 'https://gold.skia.org/_/hashes'
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
        cwd=api.vars.skia_dir,
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Run DM.
  properties = [
    'gitHash',      api.vars.got_revision,
    'master',       api.vars.master_name,
    'builder',      api.vars.builder_name,
    'build_number', api.vars.build_number,
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',         api.vars.issue,
      'patchset',      api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  if api.vars.no_buildbot:
    properties.extend(['no_buildbot', 'True'])
    properties.extend(['swarming_bot_id', api.vars.swarming_bot_id])
    properties.extend(['swarming_task_id', api.vars.swarming_task_id])

  args = [
    'dm',
    '--undefok',   # This helps branches that may not know new flags.
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

  skip_flag = None
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    skip_flag = '--nogpu'
  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    skip_flag = '--nocpu'
  if skip_flag:
    args.append(skip_flag)
  args.extend(dm_flags(api.vars.builder_name))

  env = {}
  env.update(api.vars.default_env)
  if 'Ubuntu16' in api.vars.builder_name:
    # The vulkan in this asset name simply means that the graphics driver
    # supports Vulkan. It is also the driver used for GL code.
    dri_path = api.vars.slave_dir.join('linux_vulkan_intel_driver_release')
    if 'Debug' in api.vars.builder_name:
      dri_path = api.vars.slave_dir.join('linux_vulkan_intel_driver_debug')

    if 'Vulkan' in api.vars.builder_name:
      sdk_path = api.vars.slave_dir.join('linux_vulkan_sdk', 'bin')
      lib_path = api.vars.slave_dir.join('linux_vulkan_sdk', 'lib')
      env.update({
        'PATH':'%%(PATH)s:%s' % sdk_path,
        'LD_LIBRARY_PATH': '%s:%s' % (lib_path, dri_path),
        'LIBGL_DRIVERS_PATH': dri_path,
        'VK_ICD_FILENAMES':'%s' % dri_path.join('intel_icd.x86_64.json'),
      })
    else:
      # Even the non-vulkan NUC jobs could benefit from the newer drivers.
      env.update({
        'LD_LIBRARY_PATH': dri_path,
        'LIBGL_DRIVERS_PATH': dri_path,
      })

  # See skia:2789.
  if '_AbandonGpuContext' in api.vars.builder_cfg.get('extra_config', ''):
    args.append('--abandonGpuContext')
  if '_PreAbandonGpuContext' in api.vars.builder_cfg.get('extra_config', ''):
    args.append('--preAbandonGpuContext')

  api.run(api.flavor.step, 'dm', cmd=args,
          abort_on_failure=False,
          env=env)

  if api.vars.upload_dm_results:
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.vars.dm_dir)


class TestApi(recipe_api.RecipeApi):
  def run(self):
    self.m.core.setup()
    if 'iOS' in self.m.vars.builder_name:
      self.m.vars.default_env['IOS_BUNDLE_ID'] = 'com.google.dm'
    try:
      self.m.flavor.install_everything()
      test_steps(self.m)
    finally:
      self.m.flavor.cleanup_steps()
    self.m.run.check_failure()
