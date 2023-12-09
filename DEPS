use_relative_paths = True

vars = {
  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling different
  # dependencies without interference from each other.
  'sk_tool_revision': 'git_revision:f90eb691f29a6b49887ff03fd021cc24415745f0',

  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'ninja_version': 'version:2@1.8.2.chromium.3',

  # googlefonts_testdata CIPD package version
  # https://chrome-infra-packages.appspot.com/p/chromium/third_party/googlefonts_testdata/
  'googlefonts_testdata_version': 'version:20230913',
}

# If you modify this file, you will need to regenerate the Bazel version of this file (bazel/deps.bzl).
# To do so, run:
#     bazelisk run //bazel/deps_parser
#
# To apply the changes for the GN build, you will need to resync the git repositories using:
#     ./tools/git-sync-deps
deps = {
  "buildtools"                                   : "https://chromium.googlesource.com/chromium/src/buildtools.git@b138e6ce86ae843c42a1a08f37903207bebcca75",
  "third_party/externals/angle2"                 : "https://chromium.googlesource.com/angle/angle.git@b8ca8de438417fefeb821d184322b26cb5d56a2c",
  "third_party/externals/brotli"                 : "https://skia.googlesource.com/external/github.com/google/brotli.git@6d03dfbedda1615c4cba1211f8d81735575209c8",
  "third_party/externals/d3d12allocator"         : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, tint for SPIRV compilation, and abseil for string formatting.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"                   : "https://dawn.googlesource.com/dawn.git@2291a55e72cd9c81ba668dc9553adf456bd09d37",
  "third_party/externals/jinja2"                 : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@515dd10de9bf63040045902a4a310d2ba25213a0",
  "third_party/externals/markupsafe"             : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@006709ba3ed87660a17bd4548c45663628f5ed85",
  "third_party/externals/abseil-cpp"             : "https://skia.googlesource.com/external/github.com/abseil/abseil-cpp.git@cb436cf0142b4cbe47aae94223443df7f82e2920",
  "third_party/externals/dng_sdk"                : "https://android.googlesource.com/platform/external/dng_sdk.git@c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
  "third_party/externals/egl-registry"           : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@b055c9b483e70ecd57b3cf7204db21f5a06f9ffe",
  "third_party/externals/emsdk"                  : "https://skia.googlesource.com/external/github.com/emscripten-core/emsdk.git@a896e3d066448b3530dbcaa48869fafefd738f57",
  "third_party/externals/expat"                  : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@441f98d02deafd9b090aea568282b28f66a50e36",
  "third_party/externals/freetype"               : "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git@45903920b984540bb629bc89f4c010159c23a89a",
  "third_party/externals/harfbuzz"               : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@4cfc6d8e173e800df086d7be078da2e8c5cfca19",
  "third_party/externals/highway"                : "https://chromium.googlesource.com/external/github.com/google/highway.git@424360251cdcfc314cfc528f53c872ecd63af0f0",
  "third_party/externals/icu"                    : "https://chromium.googlesource.com/chromium/deps/icu.git@a0718d4f121727e30b8d52c7a189ebf5ab52421f",
  "third_party/externals/icu4x"                  : "https://chromium.googlesource.com/external/github.com/unicode-org/icu4x.git@4f81635489681ecf7707623177123cb78d6a66a0",
  "third_party/externals/imgui"                  : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@55d35d8387c15bf0cfd71861df67af8cfbda7456",
  "third_party/externals/libavif"                : "https://skia.googlesource.com/external/github.com/AOMediaCodec/libavif.git@55aab4ac0607ab651055d354d64c4615cf3d8000",
  "third_party/externals/libgav1"                : "https://chromium.googlesource.com/codecs/libgav1.git@5cf722e659014ebaf2f573a6dd935116d36eadf1",
  "third_party/externals/libgrapheme"            : "https://skia.googlesource.com/external/github.com/FRIGN/libgrapheme/@c0cab63c5300fa12284194fbef57aa2ed62a94c0",
  "third_party/externals/libjpeg-turbo"          : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@ed683925e4897a84b3bffc5c1414c85b97a129a3",
  "third_party/externals/libjxl"                 : "https://chromium.googlesource.com/external/gitlab.com/wg1/jpeg-xl.git@a205468bc5d3a353fb15dae2398a101dff52f2d3",
  "third_party/externals/libpng"                 : "https://skia.googlesource.com/third_party/libpng.git@386707c6d19b974ca2e3db7f5c61873813c6fe44",
  "third_party/externals/libwebp"                : "https://chromium.googlesource.com/webm/libwebp.git@2af26267cdfcb63a88e5c74a85927a12d6ca1d76",
  "third_party/externals/libyuv"                 : "https://chromium.googlesource.com/libyuv/libyuv.git@d248929c059ff7629a85333699717d7a677d8d96",
  "third_party/externals/microhttpd"             : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/oboe"                   : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opengl-registry"        : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/perfetto"               : "https://android.googlesource.com/platform/external/perfetto@93885509be1c9240bc55fa515ceb34811e54a394",
  "third_party/externals/piex"                   : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sfntly"                 : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/swiftshader"            : "https://swiftshader.googlesource.com/SwiftShader@77be35e337bb80df52c5ea4dd4cf2d2072abad25",
  "third_party/externals/vulkanmemoryallocator"  : "https://chromium.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@a6bfc237255a6bac1513f7c1ebde6d8aed6b5191",
  # vulkan-deps is a meta-repo containing several interdependent Khronos Vulkan repositories.
  # When the vulkan-deps revision is updated, those repos (spirv-*, vulkan-*) should be updated as well.
  "third_party/externals/vulkan-deps"            : "https://chromium.googlesource.com/vulkan-deps@f4a64ca5f0941183abf7df8cda5ad09aab85b616",
  "third_party/externals/spirv-cross"            : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@f349c91274b91c1a7c173f2df70ec53080076191",
  "third_party/externals/spirv-headers"          : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@1c6bb2743599e6eb6f37b2969acc0aef812e32e3",
  "third_party/externals/spirv-tools"            : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@6b4f0c9d0b7d02db5ed0b03433ae62c03bbff722",
  "third_party/externals/vello"                  : "https://skia.googlesource.com/external/github.com/linebender/vello.git@ee3a076b291d206c361431cc841407adf265c692",
  "third_party/externals/vulkan-headers"         : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers@41263fc5aa994b8eafaca946583bfcceca8ca419",
  "third_party/externals/vulkan-tools"           : "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Tools@7e75f4d389799129b79f90d1401f15f511796dbd",
  "third_party/externals/vulkan-utility-libraries": "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@b89f4b8415ef6afe08a8dbb98c8487ebd59c3387",
  "third_party/externals/unicodetools"           : "https://chromium.googlesource.com/external/github.com/unicode-org/unicodetools@66a3fa9dbdca3b67053a483d130564eabc5fe095",
  #"third_party/externals/v8"                     : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"                  : "https://skia.googlesource.com/external/github.com/google/wuffs-mirror-release-c.git@e3f919ccfe3ef542cfc983a82146070258fb57f8",
  "third_party/externals/zlib"                   : "https://chromium.googlesource.com/chromium/src/third_party/zlib@c876c8f87101c5a75f6014b0f832499afeb65b73",

  'bin': {
    'packages': [
      {
        'package': 'skia/tools/sk/${{platform}}',
        'version': Var('sk_tool_revision'),
      },
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': Var('ninja_version'),
      }
    ],
    'dep_type': 'cipd',
  },
}
