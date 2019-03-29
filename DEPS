use_relative_paths = True

vars = {
  "checkout_chromium": False,
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "common"                                : "https://skia.googlesource.com/common.git@9737551d7a52c3db3262db5856e6bcd62c462b92",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@6446c8882fa3efe151b1ee451554e68d10802cc7",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@96443b262250c390b0caefbf3eed8463ba35ecae",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://android.googlesource.com/platform/external/expat.git@android-6.0.1_r55",
  "third_party/externals/freetype"        : "https://skia.googlesource.com/third_party/freetype2.git@7edc937fe679d14d66f55cf6f7fa607925d38f3c",
  "third_party/externals/harfbuzz"        : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@4f37ab63de9705d7bf74ee75364747e41b7c06a1",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@407b39301e71006b68bd38e770f35d32398a7b14",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@d38d7c6628bebd02692cfdd6fa76b4d992a35b75",
  "third_party/externals/libjpeg-turbo"   : "https://skia.googlesource.com/external/github.com/libjpeg-turbo/libjpeg-turbo.git@2.0.0",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@v1.6.33",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@v1.0.2-rc1",
  "third_party/externals/lua"             : "https://skia.googlesource.com/external/github.com/lua/lua.git@v5-3-4",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/opencl-lib"      : "https://skia.googlesource.com/external/github.com/GPUOpen-Tools/common-lib-amd-APPSDK-3.0@4e6d30e406d2e5a65e1d65e404fe6df5f772a32b",
  "third_party/externals/opencl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenCL-Registry@932ed55c85f887041291cef8019e54280c033c35",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sdl"             : "https://skia.googlesource.com/third_party/sdl@5d7cfcca344034aff9327f77fc181ae3754e7a90",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@661ad91124e6af2272afd00f804d8aa276e17107",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@e9e4393b1c5aad7553c05782acefbe32b42644bd",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@51562f15f4ad9cc7c0ea1a0e528a4f2c4aa89202",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://skia.googlesource.com/external/github.com/google/wuffs.git@ae6e8db4ad4236654e7b8c68d24f41a37e5f4f96",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@47af7c547f8551bd25424e56354a2ae1e9062859",
  "third_party/externals/Nima-Cpp"        : "https://skia.googlesource.com/external/github.com/2d-inc/Nima-Cpp.git@4bd02269d7d1d2e650950411325eafa15defb084",
  "third_party/externals/Nima-Math-Cpp"   : "https://skia.googlesource.com/external/github.com/2d-inc/Nima-Math-Cpp.git@e0c12772093fa8860f55358274515b86885f0108",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@eb8d3bc0b60753501c920029eb894f3b3b5dd7e5",
    "condition": "checkout_chromium",
  },
}

recursedeps = [
  "common",
  "../src",
]

gclient_gn_args_from = 'src'
