# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


def compile_swiftshader(api, extra_tokens, swiftshader_root, ninja_root, cc, cxx, out):
  """Build SwiftShader with CMake.

  Building SwiftShader works differently from any other Skia third_party lib.
  See discussion in skia:7671 for more detail.

  Args:
    swiftshader_root: root of the SwiftShader checkout.
    ninja_root: A folder containing a ninja binary
    cc, cxx: compiler binaries to use
    out: target directory for libvk_swiftshader.so
  """
  swiftshader_opts = [
      '-DSWIFTSHADER_BUILD_TESTS=OFF',
      '-DSWIFTSHADER_WARNINGS_AS_ERRORS=OFF',
      '-DREACTOR_ENABLE_MEMORY_SANITIZER_INSTRUMENTATION=OFF',  # Way too slow.
  ]
  cmake_bin = str(api.vars.workdir.join('cmake_linux', 'bin'))
  env = {
      'CC': cc,
      'CXX': cxx,
      'PATH': '%s:%%(PATH)s:%s' % (ninja_root, cmake_bin),
      # We arrange our MSAN/TSAN prebuilts a little differently than
      # SwiftShader's CMakeLists.txt expects, so we'll just keep our custom
      # setup (everything mentioning libcxx below) and point SwiftShader's
      # CMakeLists.txt at a harmless non-existent path.
      'SWIFTSHADER_MSAN_INSTRUMENTED_LIBCXX_PATH': '/totally/phony/path',
  }

  # Extra flags for MSAN/TSAN, if necessary.
  san = None
  if 'MSAN' in extra_tokens:
    san = ('msan','memory')

  if san:
    short,full = san
    clang_linux = str(api.vars.workdir.join('clang_linux'))
    libcxx = clang_linux + '/' + short
    cflags = ' '.join([
      '-fsanitize=' + full,
      '-stdlib=libc++',
      '-L%s/lib' % libcxx,
      '-lc++abi',
      '-I%s/include' % libcxx,
      '-I%s/include/c++/v1' % libcxx,
      '-Wno-unused-command-line-argument'  # Are -lc++abi and -Llibcxx/lib always unused?
    ])
    swiftshader_opts.extend([
      '-DSWIFTSHADER_{}=ON'.format(short.upper()),
      '-DCMAKE_C_FLAGS=%s' % cflags,
      '-DCMAKE_CXX_FLAGS=%s' % cflags,
    ])

  # Build SwiftShader.
  api.file.ensure_directory('makedirs swiftshader_out', out)
  with api.context(cwd=out, env=env):
    api.run(api.step, 'swiftshader cmake',
            cmd=['cmake'] + swiftshader_opts + [swiftshader_root, '-GNinja'])
    # See https://swiftshader-review.googlesource.com/c/SwiftShader/+/56452 for when the
    # deprecated targets were added. See skbug.com/12386 for longer-term plans.
    api.run(api.step, 'swiftshader ninja', cmd=['ninja', '-C', out, 'vk_swiftshader'])


def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  compiler      = api.vars.builder_cfg.get('compiler',      '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens  = api.vars.extra_tokens
  os            = api.vars.builder_cfg.get('os',            '')
  target_arch   = api.vars.builder_cfg.get('target_arch',   '')

  clang_linux      = str(api.vars.workdir.join('clang_linux'))
  win_toolchain    = str(api.vars.workdir.join('win_toolchain'))
  dwritecore       = str(api.vars.workdir.join('dwritecore'))

  cc, cxx, ccache = None, None, None
  extra_cflags = []
  extra_ldflags = []
  args = {'werror': 'true', 'link_pool_depth':'2'}
  env = {}

  with api.context(cwd=skia_dir):
    api.run(api.step, 'fetch-gn',
            cmd=['python3', skia_dir.join('bin', 'fetch-gn')],
            infra_step=True)

    api.run(api.step, 'fetch-ninja',
            cmd=['python3', skia_dir.join('bin', 'fetch-ninja')],
            infra_step=True)

  if os == 'Mac':
    # XCode build is listed in parentheses after the version at
    # https://developer.apple.com/news/releases/, or on Wikipedia here:
    # https://en.wikipedia.org/wiki/Xcode#Version_comparison_table
    # Use lowercase letters.
    # https://chrome-infra-packages.appspot.com/p/infra_internal/ios/xcode
    XCODE_BUILD_VERSION = '12c33'
    if compiler == 'Xcode11.4.1':
      XCODE_BUILD_VERSION = '11e503a'
    extra_cflags.append(
        '-DREBUILD_IF_CHANGED_xcode_build_version=%s' % XCODE_BUILD_VERSION)
    mac_toolchain_cmd = api.vars.workdir.join(
        'mac_toolchain', 'mac_toolchain')
    xcode_app_path = api.vars.cache_dir.join('Xcode.app')
    # Copied from
    # https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/api.py#322
    with api.step.nest('ensure xcode') as step_result:
      step_result.step_summary_text = (
          'Ensuring Xcode version %s in %s' % (
              XCODE_BUILD_VERSION, xcode_app_path))
      install_xcode_cmd = [
          mac_toolchain_cmd, 'install',
          # "ios" is needed for simulator builds
          # (Build-Mac-Clang-x64-Release-iOS).
          '-kind', 'ios',
          '-xcode-version', XCODE_BUILD_VERSION,
          '-output-dir', xcode_app_path,
      ]
      api.step('install xcode', install_xcode_cmd)
      api.step('select xcode', [
          'sudo', 'xcode-select', '-switch', xcode_app_path])
      if 'iOS' in extra_tokens:
        if compiler == 'Xcode11.4.1':
          # Ganesh has a lower minimum iOS version than Graphite but there are dedicated jobs that
          # test with the lower SDK.
          env['IPHONEOS_DEPLOYMENT_TARGET'] = '11.0'
          args['ios_min_target'] = '"11.0"'
        else:
          env['IPHONEOS_DEPLOYMENT_TARGET'] = '13.0'
          args['ios_min_target'] = '"13.0"'

      else:
        # We have some machines on 10.15.
        env['MACOSX_DEPLOYMENT_TARGET'] = '10.15'

  # ccache + clang-tidy.sh chokes on the argument list.
  if (api.vars.is_linux or os == 'Mac' or os == 'Mac10.15.5' or os == 'Mac10.15.7') and 'Tidy' not in extra_tokens:
    if api.vars.is_linux:
      ccache = api.vars.workdir.join('ccache_linux', 'bin', 'ccache')
      # As of 2020-02-07, the sum of each Debian10-Clang-x86
      # non-flutter/android/chromebook build takes less than 75G cache space.
      env['CCACHE_MAXSIZE'] = '75G'
    else:
      ccache = api.vars.workdir.join('ccache_mac', 'bin', 'ccache')
      # As of 2020-02-10, the sum of each Build-Mac-Clang- non-android build
      # takes ~30G cache space.
      env['CCACHE_MAXSIZE'] = '50G'

    args['cc_wrapper'] = '"%s"' % ccache

    env['CCACHE_DIR'] = api.vars.cache_dir.join('ccache')
    env['CCACHE_MAXFILES'] = '0'
    # Compilers are unpacked from cipd with bogus timestamps, only contribute
    # compiler content to hashes. If Ninja ever uses absolute paths to changing
    # directories we'll also need to set a CCACHE_BASEDIR.
    env['CCACHE_COMPILERCHECK'] = 'content'

  if compiler == 'Clang' and api.vars.is_linux:
    cc  = clang_linux + '/bin/clang'
    cxx = clang_linux + '/bin/clang++'
    extra_cflags .append('-B%s/bin' % clang_linux)
    extra_ldflags.append('-B%s/bin' % clang_linux)
    extra_ldflags.append('-fuse-ld=lld')
    extra_cflags.append('-DPLACEHOLDER_clang_linux_version=%s' %
                        api.run.asset_version('clang_linux', skia_dir))
    if 'Static' in extra_tokens:
      extra_ldflags.extend(['-static-libstdc++', '-static-libgcc'])

  elif compiler == 'Clang':
    cc, cxx = 'clang', 'clang++'

  if 'Tidy' in extra_tokens:
    # Swap in clang-tidy.sh for clang++, but update PATH so it can find clang++.
    cxx = skia_dir.join("tools/clang-tidy.sh")
    env['PATH'] = '%s:%%(PATH)s' % (clang_linux + '/bin')
    # Increase ClangTidy code coverage by enabling features.
    args.update({
      'skia_enable_fontmgr_empty':     'true',
      'skia_enable_graphite':          'true',
      'skia_enable_pdf':               'true',
      'skia_use_dawn':                 'true',
      'skia_use_expat':                'true',
      'skia_use_freetype':             'true',
      'skia_use_vulkan':               'true',
    })

  if 'Coverage' in extra_tokens:
    # See https://clang.llvm.org/docs/SourceBasedCodeCoverage.html for
    # more info on using llvm to gather coverage information.
    extra_cflags.append('-fprofile-instr-generate')
    extra_cflags.append('-fcoverage-mapping')
    extra_ldflags.append('-fprofile-instr-generate')
    extra_ldflags.append('-fcoverage-mapping')

  if compiler != 'MSVC' and configuration == 'Debug':
    extra_cflags.append('-O1')
  if compiler != 'MSVC' and configuration == 'OptimizeForSize':
    # build IDs are required for Bloaty if we want to use strip to ignore debug symbols.
    # https://github.com/google/bloaty/blob/master/doc/using.md#debugging-stripped-binaries
    extra_ldflags.append('-Wl,--build-id=sha1')
    args.update({
      'skia_use_runtime_icu': 'true',
      'skia_enable_optimize_size': 'true',
      'skia_use_jpeg_gainmaps': 'false',
    })

  if 'Exceptions' in extra_tokens:
    extra_cflags.append('/EHsc')
  if 'Fast' in extra_tokens:
    extra_cflags.extend(['-march=native', '-fomit-frame-pointer', '-O3',
                         '-ffp-contract=off'])

  if len(extra_tokens) == 1 and extra_tokens[0].startswith('SK'):
    extra_cflags.append('-D' + extra_tokens[0])
    # If we're limiting Skia at all, drop skcms to portable code.
    if 'SK_CPU_LIMIT' in extra_tokens[0]:
      extra_cflags.append('-DSKCMS_PORTABLE')

  if 'MSAN' in extra_tokens:
    extra_ldflags.append('-L' + clang_linux + '/msan')
  elif 'TSAN' in extra_tokens:
    extra_ldflags.append('-L' + clang_linux + '/tsan')
  elif api.vars.is_linux:
    extra_ldflags.append('-L' + clang_linux + '/lib')

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  if 'Dawn' in extra_tokens:
    util.set_dawn_args_and_env(args, env, api, extra_tokens, skia_dir)
  if 'ANGLE' in extra_tokens:
    args['skia_use_angle'] = 'true'
  if 'SwiftShader' in extra_tokens:
    swiftshader_root = skia_dir.join('third_party', 'externals', 'swiftshader')
    # Swiftshader will need to make ninja be on the path
    ninja_root = skia_dir.join('third_party', 'ninja')
    swiftshader_out = out_dir.join('swiftshader_out')
    compile_swiftshader(api, extra_tokens, swiftshader_root, ninja_root, cc, cxx, swiftshader_out)
    args['skia_use_vulkan'] = 'true'
    extra_cflags.extend(['-DSK_GPU_TOOLS_VK_LIBRARY_NAME=%s' %
        api.vars.swarming_out_dir.join('swiftshader_out', 'libvk_swiftshader.so'),
    ])
  if 'MSAN' in extra_tokens:
    args['skia_use_fontconfig'] = 'false'
  if 'ASAN' in extra_tokens:
    args['skia_enable_spirv_validation'] = 'false'
  if 'NoPrecompile' in extra_tokens:
    args['skia_enable_precompile'] = 'false'
  if 'Graphite' in extra_tokens:
    args['skia_enable_graphite'] = 'true'
  if 'Vello' in extra_tokens:
    args['skia_enable_vello_shaders'] = 'true'
  if 'Fontations' in extra_tokens:
    args['skia_use_fontations'] = 'true'
    args['skia_use_freetype'] = 'true' # we compare with freetype in tests
    args['skia_use_system_freetype2'] = 'false'
  if 'FreeType' in extra_tokens:
    args['skia_use_freetype'] = 'true'
    args['skia_use_system_freetype2'] = 'false'

  if 'NoGpu' in extra_tokens:
    args['skia_enable_ganesh'] = 'false'
  if 'NoDEPS' in extra_tokens:
    args.update({
      'is_official_build':             'true',
      'skia_enable_fontmgr_empty':     'true',
      'skia_enable_ganesh':            'true',

      'skia_enable_pdf':               'false',
      'skia_use_expat':                'false',
      'skia_use_freetype':             'false',
      'skia_use_harfbuzz':             'false',
      'skia_use_icu':                  'false',
      'skia_use_libjpeg_turbo_decode': 'false',
      'skia_use_libjpeg_turbo_encode': 'false',
      'skia_use_libpng_decode':        'false',
      'skia_use_libpng_encode':        'false',
      'skia_use_libwebp_decode':       'false',
      'skia_use_libwebp_encode':       'false',
      'skia_use_vulkan':               'false',
      'skia_use_wuffs':                'false',
      'skia_use_zlib':                 'false',
    })
  elif configuration != 'OptimizeForSize':
    args.update({
      'skia_use_client_icu': 'true',
      'skia_use_libgrapheme': 'true',
    })

  if 'Fontations' in extra_tokens:
    args['skia_use_icu4x'] = 'true'

  if 'Shared' in extra_tokens:
    args['is_component_build'] = 'true'
  if 'Vulkan' in extra_tokens and not 'Android' in extra_tokens and not 'Dawn' in extra_tokens:
    args['skia_use_vulkan'] = 'true'
    args['skia_enable_vulkan_debug_layers'] = 'true'
    # When running TSAN with Vulkan on NVidia, we experienced some timeouts. We found
    # a workaround (in GrContextFactory) that requires GL (in addition to Vulkan).
    if 'TSAN' in extra_tokens:
      args['skia_use_gl'] = 'true'
    else:
      args['skia_use_gl'] = 'false'
  if 'Direct3D' in extra_tokens and not 'Dawn' in extra_tokens:
    args['skia_use_direct3d'] = 'true'
    args['skia_use_gl'] = 'false'
  if 'Metal' in extra_tokens and not 'Dawn' in extra_tokens:
    args['skia_use_metal'] = 'true'
    args['skia_use_gl'] = 'false'
  if 'iOS' in extra_tokens:
    # Bots use Chromium signing cert.
    args['skia_ios_identity'] = '".*83FNP.*"'
    # Get mobileprovision via the CIPD package.
    args['skia_ios_profile'] = '"%s"' % api.vars.workdir.join(
        'provisioning_profile_ios',
        'Upstream_Testing_Provisioning_Profile.mobileprovision')
  if compiler == 'Clang' and 'Win' in os:
    args['clang_win'] = '"%s"' % api.vars.workdir.join('clang_win')
    extra_cflags.append('-DPLACEHOLDER_clang_win_version=%s' %
                        api.run.asset_version('clang_win', skia_dir))

  sanitize = ''
  for t in extra_tokens:
    if t.endswith('SAN'):
      sanitize = t
      if api.vars.is_linux and t == 'ASAN':
        # skia:8712 and skia:8713
        extra_cflags.append('-DSK_ENABLE_SCOPED_LSAN_SUPPRESSIONS')
  if 'SafeStack' in extra_tokens:
    assert sanitize == ''
    sanitize = 'safe-stack'

  if 'Wuffs' in extra_tokens:
    args['skia_use_wuffs'] = 'true'

  if 'AVIF' in extra_tokens:
    args['skia_use_libavif'] = 'true'

  for (k,v) in {
    'cc':  cc,
    'cxx': cxx,
    'sanitize': sanitize,
    'target_cpu': target_arch,
    'target_os': 'ios' if 'iOS' in extra_tokens else '',
    'win_sdk': win_toolchain + '/win_sdk' if 'Win' in os else '',
    'win_vc': win_toolchain + '/VC' if 'Win' in os else '',
    'skia_dwritecore_sdk': dwritecore if 'DWriteCore' in extra_tokens else '',
  }.items():
    if v:
      args[k] = '"%s"' % v
  if extra_cflags:
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  if extra_ldflags:
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.items()))
  gn = skia_dir.join('bin', 'gn')
  ninja = skia_dir.join('third_party', 'ninja', 'ninja')

  with api.context(cwd=skia_dir):
    with api.env(env):
      if ccache:
        api.run(api.step, 'ccache stats-start', cmd=[ccache, '-s'])
      api.run(api.step, 'gn gen',
              cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
      if 'Fontations' in extra_tokens:
        api.run(api.step, 'gn clean',
              cmd=[gn, 'clean', out_dir])
      api.run(api.step, 'ninja', cmd=[ninja, '-C', out_dir])
      if ccache:
        api.run(api.step, 'ccache stats-end', cmd=[ccache, '-s'])


def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
  extra_tokens  = api.vars.extra_tokens
  os            = api.vars.builder_cfg.get('os', '')
  configuration = api.vars.builder_cfg.get('configuration', '')

  if 'SwiftShader' in extra_tokens:
    util.copy_listed_files(api,
        src.join('swiftshader_out'),
        api.vars.swarming_out_dir.join('swiftshader_out'),
        util.DEFAULT_BUILD_PRODUCTS)

  if configuration == 'OptimizeForSize':
    util.copy_listed_files(api, src, dst, ['skottie_tool_cpu', 'skottie_tool_gpu'])

  if os == 'Mac' and any('SAN' in t for t in extra_tokens):
    # The XSAN dylibs are in
    # Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib
    # /clang/11.0.0/lib/darwin, where 11.0.0 could change in future versions.
    xcode_clang_ver_dirs = api.file.listdir(
        'find XCode Clang version',
        api.vars.cache_dir.join(
            'Xcode.app', 'Contents', 'Developer', 'Toolchains',
            'XcodeDefault.xctoolchain', 'usr', 'lib', 'clang'),
        test_data=['11.0.0'])
    assert len(xcode_clang_ver_dirs) == 1
    dylib_dir = xcode_clang_ver_dirs[0].join('lib', 'darwin')
    dylibs = api.file.glob_paths('find xSAN dylibs', dylib_dir,
                                 'libclang_rt.*san_osx_dynamic.dylib',
                                 test_data=[
                                     'libclang_rt.asan_osx_dynamic.dylib',
                                     'libclang_rt.tsan_osx_dynamic.dylib',
                                     'libclang_rt.ubsan_osx_dynamic.dylib',
                                 ])
    for f in dylibs:
      api.file.copy('copy %s' % api.path.basename(f), f, dst)
