use_relative_paths = True

vars = {
  "checkout_chromium": False,
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "common"                                : "https://skia.googlesource.com/common.git@9737551d7a52c3db3262db5856e6bcd62c462b92",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@f421e0e33e6656b30ddbda920395f2657c619d22",
  "third_party/externals/dawn"            : "https://dawn.googlesource.com/dawn.git@472bd1ec29d8d4e03404bf4b4e0233e4fc6031ce",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://android.googlesource.com/platform/external/expat.git@e5aa0a2cb0a5f759ef31c0819dc67d9b14246a4a",
  "third_party/externals/freetype"        : "https://skia.googlesource.com/third_party/freetype2.git@05439f5cc69eaa3deaf3db52a7999af09a2c293a",
  "third_party/externals/harfbuzz"        : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@1bada656a86e9cb27d4a6b9fcc50748f0bd9c1d9",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@407b39301e71006b68bd38e770f35d32398a7b14",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@d38d7c6628bebd02692cfdd6fa76b4d992a35b75",
  "third_party/externals/libjpeg-turbo"   : "https://skia.googlesource.com/external/github.com/libjpeg-turbo/libjpeg-turbo.git@574f3a772c96dc9db2c98ef24706feb3f6dbda9a",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@386707c6d19b974ca2e3db7f5c61873813c6fe44",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@0fe1a89dbf1930fc2554dbe76adad5d962054ead",
  "third_party/externals/lua"             : "https://skia.googlesource.com/external/github.com/lua/lua.git@e354c6355e7f48e087678ec49e340ca0696725b1",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/opencl-lib"      : "https://skia.googlesource.com/external/github.com/GPUOpen-Tools/common-lib-amd-APPSDK-3.0@4e6d30e406d2e5a65e1d65e404fe6df5f772a32b",
  "third_party/externals/opencl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenCL-Registry@932ed55c85f887041291cef8019e54280c033c35",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sdl"             : "https://skia.googlesource.com/third_party/sdl@5d7cfcca344034aff9327f77fc181ae3754e7a90",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-cross"     : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@53ab2144b90abede33be5161aec5dfc94ddc3caf",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@29c11140baaf9f7fdaa39a583672c556bf1795a1",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@0c4feb643b89d1792b02f7cbef315e9d95633bd7",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@5a5ffe52a944a4068f2297ec404b7b5e9a9b9e03",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://skia.googlesource.com/external/github.com/google/wuffs.git@6ad7d00a262e862549e4963b4a43d148a8285e50",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@47af7c547f8551bd25424e56354a2ae1e9062859",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@4712e39822ec05dfc3e3c812c333eb56155c7e4e",
    "condition": "checkout_chromium",
  },
}

recursedeps = [
  "common",
  "../src",
]

gclient_gn_args_from = 'src'
