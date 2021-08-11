use_relative_paths = True

vars = {
  "checkout_chromium": False,

  # Three lines of non-changing comments so that
  # the commit queue can handle CLs rolling different
  # dependencies without interference from each other.
  'sk_tool_revision': 'git_revision:e6aa08c77b092f547cb277cec884f36378b40d8b',
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@29270a6369bb96efd16086bf9904d98a7384a973",
  "third_party/externals/brotli"          : "https://skia.googlesource.com/external/github.com/google/brotli.git@e61745a6b7add50d380cfd7d3883dd6c62fc2c71",
  "third_party/externals/d3d12allocator"  : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, and tint for SPIRV compilation.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"            : "https://dawn.googlesource.com/dawn.git@a6ddb0e1164cd2c75dfd4d80d56a32e2ff0dffef",
  "third_party/externals/jinja2"          : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@a82a4944a7f2496639f34a89c9923be5908b80aa",
  "third_party/externals/markupsafe"      : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@0944e71f4b2cb9a871bcbe353f95e889b64a611a",
  "third_party/externals/tint"            : "https://dawn.googlesource.com/tint@bfb27f00d70a8ef571b24d43f7900b1a29708cdf",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@a28238bdeebc087071777001245df1876a11f5ee",
  "third_party/externals/freetype"        : "https://chromium.googlesource.com/chromium/src/third_party/freetype2.git@fed5521016227bf8cc4475f66450a9963568d162",
  "third_party/externals/harfbuzz"        : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@368e9578873798e2d17ed78a0474dec7d4e9d6c0",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@a0718d4f121727e30b8d52c7a189ebf5ab52421f",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@9418dcb69355558f70de260483424412c5ca2fce",
  "third_party/externals/libgifcodec"     : "https://skia.googlesource.com/libgifcodec@fd59fa92a0c86788dcdd84d091e1ce81eda06a77",
  "third_party/externals/libjpeg-turbo"   : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@24e310554f07c0fdb8ee52e3e708e4f3e9eb6e20",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@386707c6d19b974ca2e3db7f5c61873813c6fe44",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@fedac6cc69cda3e9e04b780d324cf03921fb3ff4",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/oboe"            : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-cross"     : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@bdbef7b1f3982fe99a62d076043036abe6dd6d80",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@cf653e4ca4858583802b0d1656bc934edff6bd7f",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@11cd875ed88484f93943071083b4821b4c3d2193",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@b2af6a85583d1adf61033e82eaa5d067d764ece9",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://skia.googlesource.com/external/github.com/google/wuffs.git@1b9a6cb85b9f359595bd264d4c95da5d1291adc5",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@c876c8f87101c5a75f6014b0f832499afeb65b73",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@aa692f23db429726339eacfc55de2838056ca05a",
    "condition": "checkout_chromium",
  },

  'bin': {
    'packages': [
      {
        'package': 'skia/tools/sk/${{platform}}',
        'version': Var('sk_tool_revision'),
      }
    ],
    'dep_type': 'cipd',
  },
}

recursedeps = [
  "../src",
]

gclient_gn_args_from = 'src'
