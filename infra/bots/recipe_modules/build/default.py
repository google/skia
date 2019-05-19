# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


# See mapping of Xcode version to Xcode build version here:
# https://chromium.googlesource.com/chromium/tools/build/+/master/scripts/slave/recipe_modules/ios/api.py#37
# When updating XCODE_BUILD_VERSION, you will also need to update
# XCODE_CLANG_VERSION.
XCODE_BUILD_VERSION = '9c40b'
XCODE_CLANG_VERSION = '9.0.0'


def build_command_buffer(api, chrome_dir, skia_dir, out):
  api.run(api.python, 'build command_buffer',
      script=skia_dir.join('tools', 'build_command_buffer.py'),
      args=[
        '--chrome-dir', chrome_dir,
        '--output-dir', out,
        '--no-sync', '--no-hooks', '--make-output-dir'])


def compile_swiftshader(api, swiftshader_root, cc, cxx, out):
  """Build SwiftShader with CMake.

  Building SwiftShader works differently from any other Skia third_party lib.
  See discussion in skia:7671 for more detail.

  Args:
    swiftshader_root: root of the SwiftShader checkout.
    cc, cxx: compiler binaries to use
    out: target directory for libEGL.so and libGLESv2.so
  """
  cmake_bin = str(api.vars.slave_dir.join('cmake_linux', 'bin'))
  env = {
      'CC': cc,
      'CXX': cxx,
      'PATH': '%%(PATH)s:%s' % cmake_bin
  }
  api.file.ensure_directory('makedirs swiftshader_out', out)
  with api.context(cwd=out, env=env):
    api.run(api.step, 'swiftshader cmake',
            cmd=['cmake', '-DBUILD_TESTS=OFF', '-DWARNINGS_AS_ERRORS=0', swiftshader_root, '-GNinja'])
    api.run(api.step, 'swiftshader ninja',
            cmd=['ninja', '-C', out, 'libEGL.so', 'libGLESv2.so'])


def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  compiler      = api.vars.builder_cfg.get('compiler',      '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens  = api.vars.extra_tokens
  os            = api.vars.builder_cfg.get('os',            '')
  target_arch   = api.vars.builder_cfg.get('target_arch',   '')

  clang_linux      = str(api.vars.slave_dir.join('clang_linux'))
  win_toolchain    = str(api.vars.slave_dir.join('win_toolchain'))
  moltenvk         = str(api.vars.slave_dir.join('moltenvk'))

  cc, cxx = None, None
  extra_cflags = []
  extra_ldflags = []
  args = {'werror': 'true'}
  env = {}

  if os == 'Mac':
    extra_cflags.append(
        '-DDUMMY_xcode_build_version=%s' % XCODE_BUILD_VERSION)
    mac_toolchain_cmd = api.vars.slave_dir.join(
        'mac_toolchain', 'mac_toolchain')
    xcode_app_path = api.vars.cache_dir.join('Xcode.app')
    # Copied from
    # https://chromium.googlesource.com/chromium/tools/build/+/e19b7d9390e2bb438b566515b141ed2b9ed2c7c2/scripts/slave/recipe_modules/ios/api.py#322
    with api.step.nest('ensure xcode') as step_result:
      step_result.presentation.step_text = (
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

  if compiler == 'Clang' and api.vars.is_linux:
    cc  = clang_linux + '/bin/clang'
    cxx = clang_linux + '/bin/clang++'
    extra_cflags .append('-B%s/bin' % clang_linux)
    extra_ldflags.append('-B%s/bin' % clang_linux)
    extra_ldflags.append('-fuse-ld=lld')
    extra_cflags.append('-DDUMMY_clang_linux_version=%s' %
                        api.run.asset_version('clang_linux', skia_dir))
    if 'Static' in extra_tokens:
      extra_ldflags.extend(['-static-libstdc++', '-static-libgcc'])

  elif compiler == 'Clang':
    cc, cxx = 'clang', 'clang++'
  elif compiler == 'GCC':
    if target_arch in ['mips64el', 'loongson3a']:
      mips64el_toolchain_linux = str(api.vars.slave_dir.join(
          'mips64el_toolchain_linux'))
      cc  = mips64el_toolchain_linux + '/bin/mips64el-linux-gnuabi64-gcc-7'
      cxx = mips64el_toolchain_linux + '/bin/mips64el-linux-gnuabi64-g++-7'
      env['LD_LIBRARY_PATH'] = (
          mips64el_toolchain_linux + '/lib/x86_64-linux-gnu/')
      extra_ldflags.append('-L' + mips64el_toolchain_linux +
                           '/mips64el-linux-gnuabi64/lib')
      extra_cflags.extend([
          '-Wno-format-truncation',
          '-Wno-uninitialized',
          ('-DDUMMY_mips64el_toolchain_linux_version=%s' %
           api.run.asset_version('mips64el_toolchain_linux', skia_dir))
      ])
      if configuration == 'Release':
        # This warning is only triggered when fuzz_canvas is inlined.
        extra_cflags.append('-Wno-strict-overflow')
      args.update({
        'skia_use_system_freetype2': 'false',
        'skia_use_fontconfig':       'false',
        'skia_enable_gpu':           'false',
      })
    else:
      cc, cxx = 'gcc', 'g++'

  if 'Tidy' in extra_tokens:
    # Swap in clang-tidy.sh for clang++, but update PATH so it can find clang++.
    cxx = skia_dir.join("tools/clang-tidy.sh")
    env['PATH'] = '%s:%%(PATH)s' % (clang_linux + '/bin')

  if 'Coverage' in extra_tokens:
    # See https://clang.llvm.org/docs/SourceBasedCodeCoverage.html for
    # more info on using llvm to gather coverage information.
    extra_cflags.append('-fprofile-instr-generate')
    extra_cflags.append('-fcoverage-mapping')
    extra_ldflags.append('-fprofile-instr-generate')
    extra_ldflags.append('-fcoverage-mapping')

  if compiler != 'MSVC' and configuration == 'Debug':
    extra_cflags.append('-O1')

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

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  if 'ANGLE' in extra_tokens:
    args['skia_use_angle'] = 'true'
  if 'SwiftShader' in extra_tokens:
    swiftshader_root = skia_dir.join('third_party', 'externals', 'swiftshader')
    swiftshader_out = out_dir.join('swiftshader_out')
    compile_swiftshader(api, swiftshader_root, cc, cxx, swiftshader_out)
    args['skia_use_egl'] = 'true'
    extra_cflags.extend([
        '-DGR_EGL_TRY_GLES3_THEN_GLES2',
        '-I%s' % skia_dir.join(
            'third_party', 'externals', 'egl-registry', 'api'),
        '-I%s' % skia_dir.join(
            'third_party', 'externals', 'opengl-registry', 'api'),
    ])
    extra_ldflags.extend([
        '-L%s' % swiftshader_out,
    ])
  if 'CommandBuffer' in extra_tokens:
    # CommandBuffer runs against GLES version of CommandBuffer also, so
    # include both.
    args.update({
      'skia_gl_standard': '""',
    })
    chrome_dir = checkout_root
    api.run.run_once(build_command_buffer, api, chrome_dir, skia_dir, out_dir)
  if 'MSAN' in extra_tokens:
    args['skia_use_fontconfig'] = 'false'
  if 'ASAN' in extra_tokens or 'UBSAN' in extra_tokens:
    args['skia_enable_spirv_validation'] = 'false'
  if 'NoDEPS' in extra_tokens:
    args.update({
      'is_official_build':         'true',
      'skia_enable_fontmgr_empty': 'true',
      'skia_enable_gpu':           'true',

      'skia_enable_pdf':        'false',
      'skia_use_expat':         'false',
      'skia_use_freetype':      'false',
      'skia_use_harfbuzz':      'false',
      'skia_use_libjpeg_turbo': 'false',
      'skia_use_libpng':        'false',
      'skia_use_libwebp':       'false',
      'skia_use_vulkan':        'false',
      'skia_use_zlib':          'false',
    })
  if 'NoGPU' in extra_tokens:
    args['skia_enable_gpu'] = 'false'
  if 'Shared' in extra_tokens:
    args['is_component_build'] = 'true'
  if 'Vulkan' in extra_tokens and not 'Android' in extra_tokens:
    args['skia_use_vulkan'] = 'true'
    args['skia_enable_vulkan_debug_layers'] = 'false'
    if 'MoltenVK' in extra_tokens:
      args['skia_moltenvk_path'] = '"%s"' % moltenvk
  if 'Metal' in extra_tokens:
    args['skia_use_metal'] = 'true'
  if 'OpenCL' in extra_tokens:
    args['skia_use_opencl'] = 'true'
    if api.vars.is_linux:
      extra_cflags.append(
          '-isystem%s' % api.vars.slave_dir.join('opencl_headers'))
      extra_ldflags.append(
          '-L%s' % api.vars.slave_dir.join('opencl_ocl_icd_linux'))
    elif 'Win' in os:
      extra_cflags.append(
          '-imsvc%s' % api.vars.slave_dir.join('opencl_headers'))
      extra_ldflags.append(
          '/LIBPATH:%s' %
          skia_dir.join('third_party', 'externals', 'opencl-lib', '3-0', 'lib',
                        'x86_64'))
  if 'iOS' in extra_tokens:
    # Bots use Chromium signing cert.
    args['skia_ios_identity'] = '".*GS9WA.*"'
    args['skia_ios_profile'] = '"Upstream Testing Provisioning Profile"'
  if 'CheckGeneratedFiles' in extra_tokens:
    args['skia_compile_processors'] = 'true'
    args['skia_generate_workarounds'] = 'true'
  if compiler == 'Clang' and 'Win' in os:
    args['clang_win'] = '"%s"' % api.vars.slave_dir.join('clang_win')
    extra_cflags.append('-DDUMMY_clang_win_version=%s' %
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
  if 'MSRTC' in extra_tokens:
    assert sanitize == ''
    sanitize = 'MSVC'

  if 'Wuffs' in extra_tokens:
    args['skia_use_wuffs'] = 'true'

  for (k,v) in {
    'cc':  cc,
    'cxx': cxx,
    'sanitize': sanitize,
    'target_cpu': target_arch,
    'target_os': 'ios' if 'iOS' in extra_tokens else '',
    'win_sdk': win_toolchain + '/win_sdk' if 'Win' in os else '',
    'win_vc': win_toolchain + '/VC' if 'Win' in os else '',
  }.iteritems():
    if v:
      args[k] = '"%s"' % v
  if extra_cflags:
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  if extra_ldflags:
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))
  gn = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir):
    api.run(api.python,
            'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)
    if 'CheckGeneratedFiles' in extra_tokens:
      env['PATH'] = '%s:%%(PATH)s' % skia_dir.join('bin')
      api.run(api.python,
              'fetch-clang-format',
              script=skia_dir.join('bin', 'fetch-clang-format'),
              infra_step=True)

    with api.env(env):
      api.run(api.step, 'gn gen',
              cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
      api.run(api.step, 'ninja', cmd=['ninja', '-C', out_dir])


def copy_extra_build_products(api, src, dst):
  extra_tokens  = api.vars.extra_tokens
  os            = api.vars.builder_cfg.get('os', '')

  if 'SwiftShader' in extra_tokens:
    util.copy_whitelisted_build_products(api,
        src.join('swiftshader_out'),
        api.vars.swarming_out_dir.join('swiftshader_out'))

  if os == 'Mac' and any('SAN' in t for t in extra_tokens):
    # Hardcoding this path because it should only change when we upgrade to a
    # new Xcode.
    lib_dir = api.vars.cache_dir.join(
        'Xcode.app', 'Contents', 'Developer', 'Toolchains',
        'XcodeDefault.xctoolchain', 'usr', 'lib', 'clang', XCODE_CLANG_VERSION,
        'lib', 'darwin')
    dylibs = api.file.glob_paths('find xSAN dylibs', lib_dir,
                                 'libclang_rt.*san_osx_dynamic.dylib',
                                 test_data=[
                                     'libclang_rt.asan_osx_dynamic.dylib',
                                     'libclang_rt.tsan_osx_dynamic.dylib',
                                     'libclang_rt.ubsan_osx_dynamic.dylib',
                                 ])
    for f in dylibs:
      api.file.copy('copy %s' % api.path.basename(f), f, dst)
