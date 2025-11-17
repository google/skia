# This file manages the dependencies used by Skia developers for local, stand-alone Skia builds and
# Skia testing infrastructure. The versions specified by the commit hashes represent the revisions
# we happen to be currently testing. Skia provides no endorsement or recommendation on the revision
# to use for these libraries.

use_relative_paths = True

vars = {
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling different
  # dependencies without interference from each other.
  'infra_revision': '00165670a412539aaa1ab1205e9da0473260d24d',

  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'ninja_version': 'version:2@1.12.1.chromium.4',

  # googlefonts_testdata CIPD package version
  # https://chrome-infra-packages.appspot.com/p/chromium/third_party/googlefonts_testdata/
  'googlefonts_testdata_version': 'version:20230913',

  # Pre-built task drivers from this repo, used for CI.
  'task_drivers_revision': 'git_revision:b5d31abb7bc772a69f800de45783768768437675',
}

# If you modify this file, you will need to regenerate the Bazel version of this file (bazel/deps.bzl).
# To do so, run:
#     bazelisk run //bazel/deps_parser
#
# To apply the changes for the GN build, you will need to resync the git repositories using:
#     ./tools/git-sync-deps
deps = {
  "buildtools"                                   : "https://chromium.googlesource.com/chromium/src/buildtools.git@729495f2ffa69080907780591fa2a630b2556e98",
  "third_party/externals/angle2"                 : "https://chromium.googlesource.com/angle/angle.git@127a84404b88dbc4327ffb7f831a9a36c3b111bc",
  "third_party/externals/brotli"                 : "https://skia.googlesource.com/external/github.com/google/brotli.git@6d03dfbedda1615c4cba1211f8d81735575209c8",
  "third_party/externals/d3d12allocator"         : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, abseil for string formatting,
  # and a few other depencencies.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"                   : "https://dawn.googlesource.com/dawn.git@f872a119cc10bb099169df1965e313086518b7e4",
  "third_party/externals/abseil-cpp"             : "https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp@e1655ca1acab4bf3f4f293ac0e14a8ddec440332",
  "third_party/externals/jinja2"                 : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@e2d024354e11cc6b041b0cff032d73f0c7e43a07",
  "third_party/externals/markupsafe"             : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@0bad08bb207bbfc1d6f3bbc82b9242b0c50e5794",
  'third_party/externals/glslang'                : 'https://chromium.googlesource.com/external/github.com/KhronosGroup/glslang@1d47ffa8ac4374a19b302021e216a20f22a3de92',
  'third_party/externals/webgpu-headers'         : 'https://chromium.googlesource.com/external/github.com/webgpu-native/webgpu-headers@706853a9da45b8e89b7ea005aa267294d115f8ce',

  "third_party/externals/delaunator-cpp"         : "https://skia.googlesource.com/external/github.com/skia-dev/delaunator-cpp.git@98305ef6c4e862f7d48df9cc647b690d796fec68",
  "third_party/externals/dng_sdk"                : "https://android.googlesource.com/platform/external/dng_sdk.git@dbe0a676450d9b8c71bf00688bb306409b779e90",
  "third_party/externals/egl-registry"           : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@b055c9b483e70ecd57b3cf7204db21f5a06f9ffe",
  "third_party/externals/emsdk"                  : "https://skia.googlesource.com/external/github.com/emscripten-core/emsdk.git@c69d433d8509c5c64564c2f0d054bf102a5cf67e",
  "third_party/externals/expat"                  : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@8e49998f003d693213b538ef765814c7d21abada",
  "third_party/externals/freetype"               : "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git@1518bc83d26b434031bd12c706ac3c7dab3902fd",
  "third_party/externals/harfbuzz"               : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@08b52ae2e44931eef163dbad71697f911fadc323",
  "third_party/externals/highway"                : "https://chromium.googlesource.com/external/github.com/google/highway.git@424360251cdcfc314cfc528f53c872ecd63af0f0",
  "third_party/externals/icu"                    : "https://chromium.googlesource.com/chromium/deps/icu.git@364118a1d9da24bb5b770ac3d762ac144d6da5a4",
  "third_party/externals/icu4x"                  : "https://chromium.googlesource.com/external/github.com/unicode-org/icu4x.git@bcf4f7198d4dc5f3127e84a6ca657c88e7d07a13",
  "third_party/externals/imgui"                  : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@55d35d8387c15bf0cfd71861df67af8cfbda7456",
  "third_party/externals/libavif"                : "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git@55aab4ac0607ab651055d354d64c4615cf3d8000",
  "third_party/externals/libgav1"                : "https://chromium.googlesource.com/codecs/libgav1.git@5cf722e659014ebaf2f573a6dd935116d36eadf1",
  "third_party/externals/libgrapheme"            : "https://skia.googlesource.com/external/github.com/FRIGN/libgrapheme/@c0cab63c5300fa12284194fbef57aa2ed62a94c0",
  "third_party/externals/libjpeg-turbo"          : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@e14cbfaa85529d47f9f55b0f104a579c1061f9ad",
  "third_party/externals/libjxl"                 : "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git@a205468bc5d3a353fb15dae2398a101dff52f2d3",
  "third_party/externals/libpng"                 : "https://skia.googlesource.com/third_party/libpng.git@ed217e3e601d8e462f7fd1e04bed43ac42212429",
  "third_party/externals/libwebp"                : "https://chromium.googlesource.com/webm/libwebp.git@845d5476a866141ba35ac133f856fa62f0b7445f",
  "third_party/externals/libyuv"                 : "https://chromium.googlesource.com/libyuv/libyuv.git@d248929c059ff7629a85333699717d7a677d8d96",
  "third_party/externals/oboe"                   : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opengl-registry"        : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/partition_alloc"        : "https://chromium.googlesource.com/chromium/src/base/allocator/partition_allocator.git@ce13777cb731e0a60c606d1741091fd11a0574d7",
  "third_party/externals/perfetto"               : "https://android.googlesource.com/platform/external/perfetto@93885509be1c9240bc55fa515ceb34811e54a394",
  "third_party/externals/piex"                   : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/swiftshader"            : "https://swiftshader.googlesource.com/SwiftShader@f474b0ce14a6e466ef84c510d9b779c74341bc3d",
  "third_party/externals/vulkanmemoryallocator"  : "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@a6bfc237255a6bac1513f7c1ebde6d8aed6b5191",
  # vulkan-deps is a meta-repo containing several interdependent Khronos Vulkan repositories.
  # When the vulkan-deps revision is updated, those repos (spirv-*, vulkan-*) should be updated as well.
  "third_party/externals/vulkan-deps"            : "https://chromium.googlesource.com/vulkan-deps@1012b9b17dd189e3e872307212beca4b6520bf63",
  "third_party/externals/spirv-cross"            : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@b8fcf307f1f347089e3c46eb4451d27f32ebc8d3",
  "third_party/externals/spirv-headers"          : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@b824a462d4256d720bebb40e78b9eb8f78bbb305",
  "third_party/externals/spirv-tools"            : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@2c1fd8974f5d9c04629fd51bd4468fce5fed104d",
  "third_party/externals/vello"                  : "https://skia.googlesource.com/external/github.com/linebender/vello.git@3ee3bea02164c5a816fe6c16ef4e3a810edb7620",
  "third_party/externals/vulkan-headers"         : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers@39c50d7bf094853a1f9a2e8a7e3377d425ae0c6a",
  "third_party/externals/vulkan-tools"           : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools@0a1fb7e8cb346f69862e4f12c1d7b09d23e2f84c",
  "third_party/externals/vulkan-utility-libraries": "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@3249c4eedf225c113c6a341b0dc08d3681716895",
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
}
