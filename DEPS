# This file manages the dependencies used by Skia developers for local, stand-alone Skia builds and
# Skia testing infrastructure. The versions specified by the commit hashes represent the revisions
# we happen to be currently testing. Skia provides no endorsement or recommendation on the revision
# to use for these libraries.

use_relative_paths = True

vars = {
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling different
  # dependencies without interference from each other.
  'infra_revision': 'd02b5721be66280d27944ab78c24e00e80be4e48',

  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'ninja_version': 'version:2@1.12.1.chromium.4',

  # googlefonts_testdata CIPD package version
  # https://chrome-infra-packages.appspot.com/p/chromium/third_party/googlefonts_testdata/
  'googlefonts_testdata_version': 'version:20230913',

  # Pre-built task drivers from this repo, used for CI.
  'task_drivers_revision': 'git_revision:b5d31abb7bc772a69f800de45783768768437675',

  'checkout_agents_internal': False,
}

# If you modify this file, you will need to regenerate the Bazel version of this file (bazel/deps.bzl).
# To do so, run:
#     bazelisk run //bazel/deps_parser
#
# To apply the changes for the GN build, you will need to resync the git repositories using:
#     ./tools/git-sync-deps
deps = {
  "buildtools"                                   : "https://chromium.googlesource.com/chromium/src/buildtools.git@729495f2ffa69080907780591fa2a630b2556e98",
  "third_party/externals/angle2"                 : "https://chromium.googlesource.com/angle/angle.git@5c2e15eebf4e811de5f4b332d461fe05997d376e",
  "third_party/externals/brotli"                 : "https://skia.googlesource.com/external/github.com/google/brotli.git@6d03dfbedda1615c4cba1211f8d81735575209c8",
  "third_party/externals/d3d12allocator"         : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, abseil for string formatting,
  # and a few other depencencies.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"                   : "https://dawn.googlesource.com/dawn.git@ea7f96fc628ed541d905a7e073de251d1cf9ec14",
  "third_party/externals/abseil-cpp"             : "https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp@27f40589d91de466e489a93999e7ac087ca5a5bb",
  "third_party/externals/jinja2"                 : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@c3027d884967773057bf74b957e3fea87e5df4d7",
  "third_party/externals/markupsafe"             : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@4256084ae14175d38a3ff7d739dca83ae49ccec6",
  'third_party/externals/glslang'                : 'https://chromium.googlesource.com/external/github.com/KhronosGroup/glslang@1d47ffa8ac4374a19b302021e216a20f22a3de92',
  'third_party/externals/webgpu-headers'         : 'https://chromium.googlesource.com/external/github.com/webgpu-native/webgpu-headers@706853a9da45b8e89b7ea005aa267294d115f8ce',

  "third_party/externals/delaunator-cpp"         : "https://skia.googlesource.com/external/github.com/skia-dev/delaunator-cpp.git@98305ef6c4e862f7d48df9cc647b690d796fec68",
  "third_party/externals/dng_sdk"                : "https://android.googlesource.com/platform/external/dng_sdk.git@dbe0a676450d9b8c71bf00688bb306409b779e90",
  "third_party/externals/egl-registry"           : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@b055c9b483e70ecd57b3cf7204db21f5a06f9ffe",
  "third_party/externals/emsdk"                  : "https://skia.googlesource.com/external/github.com/emscripten-core/emsdk.git@c69d433d8509c5c64564c2f0d054bf102a5cf67e",
  "third_party/externals/expat"                  : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@6154446fccefbf3ca644894f598969113b0c7bcd",
  "third_party/externals/freetype"               : "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git@264b5fbf5b912b39f98d038bf75d39be0a73f21b",
  "third_party/externals/harfbuzz"               : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@9cb1fee51069b206effb4736e443b038d230789d",
  "third_party/externals/highway"                : "https://chromium.googlesource.com/external/github.com/google/highway.git@457c891775a7397bdb0376bb1031e6e027af1c48",
  "third_party/externals/icu"                    : "https://chromium.googlesource.com/chromium/deps/icu.git@d578f2e8b7bd5938e21cfb6bf15c079e0aa5b738",
  "third_party/externals/icu4x"                  : "https://chromium.googlesource.com/external/github.com/unicode-org/icu4x.git@bcf4f7198d4dc5f3127e84a6ca657c88e7d07a13",
  "third_party/externals/imgui"                  : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@55d35d8387c15bf0cfd71861df67af8cfbda7456",
  "third_party/externals/libavif"                : "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git@55aab4ac0607ab651055d354d64c4615cf3d8000",
  "third_party/externals/libgav1"                : "https://chromium.googlesource.com/codecs/libgav1.git@5cf722e659014ebaf2f573a6dd935116d36eadf1",
  "third_party/externals/libgrapheme"            : "https://skia.googlesource.com/external/github.com/FRIGN/libgrapheme/@c0cab63c5300fa12284194fbef57aa2ed62a94c0",
  "third_party/externals/libjpeg-turbo"          : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@e14cbfaa85529d47f9f55b0f104a579c1061f9ad",
  "third_party/externals/libjxl"                 : "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git@332feb17d17311c748445f7ee75c4fb55cc38530",
  "third_party/externals/libpng"                 : "https://skia.googlesource.com/third_party/libpng.git@d5515b5b8be3901aac04e5bd8bd5c89f287bcd33",
  "third_party/externals/libwebp"                : "https://chromium.googlesource.com/webm/libwebp.git@845d5476a866141ba35ac133f856fa62f0b7445f",
  "third_party/externals/libyuv"                 : "https://chromium.googlesource.com/libyuv/libyuv.git@d248929c059ff7629a85333699717d7a677d8d96",
  "third_party/externals/oboe"                   : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opengl-registry"        : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/partition_alloc"        : "https://chromium.googlesource.com/chromium/src/base/allocator/partition_allocator.git@b1d0141bcecfda2bfd108882d818fc5df70ae5c7",
  "third_party/externals/perfetto"               : "https://android.googlesource.com/platform/external/perfetto@93885509be1c9240bc55fa515ceb34811e54a394",
  "third_party/externals/piex"                   : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/swiftshader"            : "https://swiftshader.googlesource.com/SwiftShader@a7c547b55474c3d8bde53711eae24ae0e28bbc0a",
  "third_party/externals/vulkanmemoryallocator"  : "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@eb744ea7a2b17040121b4bbb4d6f9e8a77e3cae7",
  # vulkan-deps is a meta-repo containing several interdependent Khronos Vulkan repositories.
  # When the vulkan-deps revision is updated, those repos (spirv-*, vulkan-*) should be updated as well.
  "third_party/externals/vulkan-deps"            : "https://chromium.googlesource.com/vulkan-deps@e5ed54df51f5e835e9381b06b366c07fe9bd6b54",
  "third_party/externals/spirv-cross"            : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@b8fcf307f1f347089e3c46eb4451d27f32ebc8d3",
  "third_party/externals/spirv-headers"          : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@29981f65241605e08b0ede4cfeb999fe3b723c6a",
  "third_party/externals/spirv-tools"            : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@0d6fd73ca73830ccab5fa1f00ed5ed40124e2c55",
  "third_party/externals/vello"                  : "https://skia.googlesource.com/external/github.com/linebender/vello.git@3ee3bea02164c5a816fe6c16ef4e3a810edb7620",
  "third_party/externals/vulkan-headers"         : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers@e3b1eec08173d6b825cd3ac88c885a63b621504a",
  "third_party/externals/vulkan-tools"           : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools@8c66b352925cb771f793a4d3220b1321ae0febf1",
  "third_party/externals/vulkan-utility-libraries": "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@c279fa4350059faac3d2365df0538977e7e5b097",
  "third_party/externals/unicodetools"           : "https://chromium.googlesource.com/external/github.com/unicode-org/unicodetools@66a3fa9dbdca3b67053a483d130564eabc5fe095",
  #"third_party/externals/v8"                     : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"                  : "https://skia.googlesource.com/external/github.com/google/wuffs-mirror-release-c.git@e3f919ccfe3ef542cfc983a82146070258fb57f8",
  "third_party/externals/zlib"                   : "https://chromium.googlesource.com/chromium/src/third_party/zlib@646b7f569718921d7d4b5b8e22572ff6c76f2596",

  'bin': {
    'packages': [
      {
        'package': 'skia/tools/sk/${{platform}}',
        'version': 'git_revision:' + Var('infra_revision'),
      },
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': Var('ninja_version'),
      }
    ],
    'dep_type': 'cipd',
  },

  'task_drivers': {
    'packages': [
      {
        'package': 'skia/tools/bazel_build/${{platform}}',
        'version': Var('task_drivers_revision'),
      },
    ],
    'dep_type': 'cipd',
    'condition': 'False',
  },

  'infra/skia-infra': {
    'url': 'https://skia.googlesource.com/buildbot.git@' + Var('infra_revision'),
    'condition': 'False',
  },

  'agents/shared': 'https://chromium.googlesource.com/chromium/agents/@e75efa515896f6bf1dea92eaffbcf8ee711a65d8',

  'agents/internal': {
    'url': 'https://chrome-internal.googlesource.com/chrome/agents-internal/@11c700b10e171091b4f0f3cf3bf95f13dee85c93',
    'condition': 'checkout_agents_internal',
  },
}
