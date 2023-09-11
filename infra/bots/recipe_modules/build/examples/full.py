# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = 'PY3'

DEPS = [
  'build',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.vars.cache_dir.join('work')
  out_dir = checkout_root.join(
      'skia', 'out', api.vars.builder_name, api.vars.configuration)
  api.build(checkout_root=checkout_root, out_dir=out_dir)
  dst = api.vars.swarming_out_dir.join('out', api.vars.configuration)
  api.build.copy_build_products(out_dir=out_dir, dst=dst)
  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian10-Clang-arm-Release-Android_API26',
  'Build-Debian10-Clang-arm-Release-Android_ASAN',
  'Build-Debian10-Clang-arm-OptimizeForSize-Android_NoPatch',
  'Build-Debian10-Clang-arm-Release-Chromebook_GLES',
  'Build-Debian10-Clang-arm64-Debug-Android_FrameworkWorkarounds',
  'Build-Debian10-Clang-arm64-Debug-Android_Graphite_Vulkan',
  'Build-Debian10-Clang-arm64-Debug-Android_HWASAN',
  'Build-Debian10-Clang-arm64-Release-Android_Graphite_Dawn',
  'Build-Debian10-Clang-arm64-Release-Android_Wuffs',
  'Build-Debian10-Clang-x86_64-Debug-ASAN_Graphite_Vulkan',
  'Build-Debian10-Clang-x86_64-Debug-AVIF',
  'Build-Debian10-Clang-x86_64-Debug-Chromebook_GLES',
  'Build-Debian10-Clang-x86_64-Debug-Coverage',
  'Build-Debian10-Clang-x86_64-Debug-Fontations',
  'Build-Debian10-Clang-x86_64-Debug-MSAN',
  'Build-Debian10-Clang-x86_64-Debug-SK_CPU_LIMIT_SSE41',
  'Build-Debian10-Clang-x86_64-Debug-SafeStack',
  'Build-Debian10-Clang-x86_64-Debug-SwiftShader_Graphite',
  'Build-Debian10-Clang-x86_64-Debug-SwiftShader_MSAN',
  'Build-Debian10-Clang-x86_64-Debug-TSAN',
  'Build-Debian10-Clang-x86_64-Debug-Tidy',
  'Build-Debian10-Clang-x86_64-Debug-Vulkan_TSAN',
  'Build-Debian10-Clang-x86_64-Debug-Wuffs',
  'Build-Debian10-Clang-x86_64-OptimizeForSize',
  'Build-Debian10-Clang-x86_64-Release-ANGLE',
  'Build-Debian10-Clang-x86_64-Release-ASAN',
  'Build-Debian10-Clang-x86_64-Release-AVIF',
  'Build-Debian10-Clang-x86_64-Release-CMake',
  'Build-Debian10-Clang-x86_64-Release-Fast',
  'Build-Debian10-Clang-x86_64-Release-NoDEPS',
  'Build-Debian10-Clang-x86_64-Release-Static',
  'Build-Debian10-Clang-x86_64-Release-SwiftShader',
  'Build-Debian10-Clang-x86_64-Release-Vulkan',
  'Build-Debian10-EMCC-asmjs-Debug-PathKit',
  'Build-Debian10-EMCC-asmjs-Release-PathKit',
  'Build-Debian10-EMCC-wasm-Debug-CanvasKit',
  'Build-Debian10-EMCC-wasm-Debug-PathKit',
  'Build-Debian10-EMCC-wasm-Release-CanvasKit_CPU',
  'Build-Debian10-EMCC-wasm-Release-PathKit',
  'Build-Debian11-GCC-x86-Debug-Docker',
  'Build-Debian11-GCC-x86_64-Debug-Docker',
  'Build-Debian11-GCC-x86_64-Release-NoGPU_Docker',
  'Build-Debian11-GCC-x86_64-Release-Shared_Docker',
  'Build-Mac-Clang-arm64-Debug-Android_Vulkan',
  'Build-Mac-Clang-arm64-Debug-iOS',
  'Build-Mac-Clang-arm64-Debug-Graphite_Dawn',
  'Build-Mac-Clang-arm64-Debug-Graphite_Dawn_NoGpu',
  'Build-Mac-Clang-arm64-Debug-Graphite_Dawn_NoPrecompile',
  'Build-Mac-Clang-arm64-Debug-Graphite_Metal',
  'Build-Mac-Clang-arm64-Debug-Graphite_Metal_NoGpu',
  'Build-Mac-Clang-arm64-Debug-Graphite_Metal_NoPrecompile',
  'Build-Mac-Clang-arm64-Release-Graphite_Metal',
  'Build-Mac-Clang-arm64-Release-Graphite_Dawn',
  'Build-Mac-Clang-x86_64-Release-Graphite_Metal_Vello',
  'Build-Mac-Xcode11.4.1-arm64-Debug-iOS',
  'Build-Mac-Clang-x86_64-Debug-ASAN',
  'Build-Mac-Clang-x86_64-Debug-Metal',
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-Clang-x86-Debug-Exceptions',
  'Build-Win-Clang-x86_64-Debug-ANGLE',
  'Build-Win-Clang-x86_64-Release-Graphite_Vulkan',
  'Build-Win-Clang-x86_64-Release-Direct3D',
  'Build-Win-Clang-x86_64-Release-Shared',
  'Build-Win-Clang-x86_64-Release-Vulkan',
]

# Default properties used for TEST_BUILDERS.
defaultProps = lambda buildername: dict(
  buildername=buildername,
  repository='https://skia.googlesource.com/skia.git',
  revision='abc123',
  path_config='kitchen',
  patch_set=2,
  swarm_out_dir='[SWARM_OUT_DIR]'
)

def GenTests(api):
  for buildername in TEST_BUILDERS:
    test = (
      api.test(buildername) +
      api.properties(**defaultProps(buildername))
    )
    if 'Win' in buildername:
      test += api.platform('win', 64)
    yield test

  yield (
      api.test('unknown-docker-image') +
      api.properties(**defaultProps('Build-Unix-GCC-x86_64-Release-Docker')) +
      api.expect_exception('Exception')
  )
