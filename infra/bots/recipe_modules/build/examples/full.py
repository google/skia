# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'build',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.vars.cache_dir.join('work')
  compiler      = api.properties['compiler']
  configuration = api.properties['configuration']
  extra_tokens  = api.properties['extra_tokens'].split(',')
  os            = api.properties['os']
  target_arch   = api.properties['target_arch']
  if ('Win' in os and target_arch == 'x86_64'):
      configuration += '_x64'

  out_dir = checkout_root.join('skia', 'out', api.vars.builder_name,
                               configuration)
  api.build(checkout_root=checkout_root, out_dir=out_dir, compiler=compiler,
            configuration=configuration, os=os, target_arch=target_arch,
            extra_tokens=extra_tokens)
  dst = api.vars.swarming_out_dir.join('out', configuration)
  api.build.copy_build_products(out_dir=out_dir, dst=dst)
  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian9-Clang-arm-Release-Android_API26',
  'Build-Debian9-Clang-arm-Release-Android_ASAN',
  'Build-Debian9-Clang-arm-Release-Chromebook_GLES',
  'Build-Debian9-Clang-universal-devrel-Android_SKQP',
  'Build-Debian9-Clang-x86_64-Debug-Chromebook_GLES',
  'Build-Debian9-Clang-x86_64-Debug-Coverage',
  'Build-Debian9-Clang-x86_64-Debug-MSAN',
  'Build-Debian9-Clang-x86_64-Debug-SK_CPU_LIMIT_SSE41',
  'Build-Debian9-Clang-x86_64-Debug-SafeStack',
  'Build-Debian9-Clang-x86_64-Release-ASAN',
  'Build-Debian9-Clang-x86_64-Release-Fast',
  'Build-Debian9-Clang-x86_64-Release-Mini',
  'Build-Debian9-Clang-x86_64-Release-NoDEPS',
  'Build-Debian9-Clang-x86_64-Release-Static',
  'Build-Debian9-Clang-x86_64-Release-SwiftShader',
  'Build-Debian9-Clang-x86_64-Release-Vulkan',
  'Build-Debian9-EMCC-wasm-Release',
  'Build-Debian9-GCC-arm-Release-Chromecast',
  'Build-Debian9-GCC-loongson3a-Release',
  'Build-Debian9-GCC-x86_64-Release-ANGLE',
  'Build-Debian9-GCC-x86_64-Release-Flutter_Android',
  'Build-Debian9-GCC-x86_64-Release-NoGPU',
  'Build-Debian9-GCC-x86_64-Release-Shared',
  'Build-Mac-Clang-arm64-Debug-Android_Vulkan',
  'Build-Mac-Clang-arm64-Debug-iOS',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Mac-Clang-x86_64-Debug-Metal',
  'Build-Mac-Clang-x86_64-Release-MoltenVK_Vulkan',
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-Clang-x86-Debug-Exceptions',
  'Build-Win-Clang-x86_64-Release-Vulkan',
  'Housekeeper-PerCommit-CheckGeneratedFiles',
]

# Default properties used for TEST_BUILDERS.
def defaultProps(buildername):
  split = buildername.split('-')
  if split[0] == 'Build':
    compiler = split[2]
    configuration = split[4]
    os = split[1]
    target_arch = split[3]

    extra_tokens_list = []
    if len(split) > 5:
      extra_split = split[5].split('_')
      for idx, tok in enumerate(extra_split):
        if tok == 'SK':
          extra_tokens_list.append('_'.join(extra_split[idx:]))
          break
        else:
          extra_tokens_list.append(tok)
    extra_tokens = ','.join(extra_tokens_list)
  else:
    compiler = ''
    configuration = 'Release'
    os = 'Debian9'
    target_arch = ''
    extra_tokens = 'CheckGeneratedFiles'

  return dict(
    buildername=buildername,
    compiler=compiler,
    configuration=configuration,
    extra_tokens=extra_tokens,
    os=os,
    repository='https://skia.googlesource.com/skia.git',
    revision='abc123',
    target_arch=target_arch,
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
    yield test

