use_relative_paths = True

vars = {
  "checkout_chromium": False,
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "common"                                : "https://skia.googlesource.com/common.git@9737551d7a52c3db3262db5856e6bcd62c462b92",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@18af9a5a50c69c72a08fcad31bd94b56b05b4834",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@96443b262250c390b0caefbf3eed8463ba35ecae",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://android.googlesource.com/platform/external/expat.git@android-6.0.1_r55",
  "third_party/externals/freetype"        : "https://skia.googlesource.com/third_party/freetype2.git@7edc937fe679d14d66f55cf6f7fa607925d38f3c",
  "third_party/externals/harfbuzz"        : "https://skia.googlesource.com/third_party/harfbuzz.git@8be74d85534534dbdd39a0a6f496e26e9f3e661d",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@407b39301e71006b68bd38e770f35d32398a7b14",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@bc6ac8b2aee0614debd940e45bc9cd0d9b355c86",
  # TODO: remove jsoncpp after migrating clients to SkJSON
  "third_party/externals/jsoncpp"         : "https://chromium.googlesource.com/external/github.com/open-source-parsers/jsoncpp.git@1.0.0",
  "third_party/externals/libjpeg-turbo"   : "https://skia.googlesource.com/external/github.com/libjpeg-turbo/libjpeg-turbo.git@2.0.0",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@v1.6.33",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@v1.0.1",
  "third_party/externals/lua"             : "https://skia.googlesource.com/external/github.com/lua/lua.git@v5-3-4",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/opencl-lib"      : "https://skia.googlesource.com/external/github.com/GPUOpen-Tools/common-lib-amd-APPSDK-3.0@4e6d30e406d2e5a65e1d65e404fe6df5f772a32b",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sdl"             : "https://skia.googlesource.com/third_party/sdl@5d7cfcca344034aff9327f77fc181ae3754e7a90",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@661ad91124e6af2272afd00f804d8aa276e17107",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@e9e4393b1c5aad7553c05782acefbe32b42644bd",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@24e71928441e008fc49485cc795f578552df5bcc",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://github.com/google/wuffs.git@bdd31a2dc880d06c6ec8f4c64ca94074d0427350",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@47af7c547f8551bd25424e56354a2ae1e9062859",
  "third_party/externals/Nima-Cpp"      : "https://github.com/2d-inc/Nima-Cpp.git@4bd02269d7d1d2e650950411325eafa15defb084",
  "third_party/externals/Nima-Math-Cpp" : "https://github.com/2d-inc/Nima-Math-Cpp.git@e0c12772093fa8860f55358274515b86885f0108",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@df35166fd2ae545e9d31701a2d9b9cb286dc5ad6",
    "condition": "checkout_chromium",
  },
}

recursedeps = [
  "common",
  "../src",
]

gclient_gn_args_from = 'src'
