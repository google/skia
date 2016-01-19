use_relative_paths = True

# Dependencies on outside packages.
#
deps = {
  "common": "https://skia.googlesource.com/common.git@c282fe0b6e392b14f88d647cbd86e1a3ef5498e0",

  # There is some duplication here that might be worth cleaning up:
  #   - can use use our existing t_p/e/libjpeg instead of pulling it for Android?

  "third_party/externals/angle2"  : "https://chromium.googlesource.com/angle/angle.git@8182049470aa3ba0c9f7c54ef965bf1877df08cb",
  "third_party/externals/cmake"   : "https://cmake.googlesource.com/cmake.git@v3.3.1",
  "third_party/externals/freetype": "https://skia.googlesource.com/third_party/freetype2.git@6a19a7d332c5446542196e5aeda0ede109ef097b",
  "third_party/externals/gyp"     : "https://chromium.googlesource.com/external/gyp.git@08429da7955a98ef764fafa223dd7de73f654b2d",
  "third_party/externals/harfbuzz": "https://skia.googlesource.com/third_party/harfbuzz.git@0.9.35",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/github.com/open-source-parsers/jsoncpp.git@1.0.0",
  # Slightly ahead of v 0.4.3, to include the fix for https://bug.skia.org/4038
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@5ff0079ece626f122bfb8e33a5f92b5a68484176",
  "third_party/externals/nanomsg" : "https://skia.googlesource.com/third_party/nanomsg.git@0.4-beta",
  "third_party/externals/zlib"    : "https://chromium.googlesource.com/chromium/src/third_party/zlib@c4e33043fb071b6ea0a153845da625d7ed633d3d",
  # NOTE: If we update libpng, we may need to update the generated file at third_party/libpng/pnglibconf.h
  "third_party/externals/libpng"  : "https://skia.googlesource.com/third_party/libpng.git@070a616b8275277e18ef8ee91e2ca23f7bdc67d5",
  "third_party/externals/giflib"  : "https://android.googlesource.com/platform/external/giflib.git@ab10e256df4f684260ca239905b1cec727181f6c",

  "third_party/externals/libjpeg-turbo"             : "https://skia.googlesource.com/third_party/libjpeg-turbo.git@36422d9e165a33914436068536772cc6ed1e7886",
  # libjpeg-turbo depends on yasm to compile .asm files
  "third_party/externals/yasm/source/patched-yasm/" : "https://chromium.googlesource.com/chromium/deps/yasm/patched-yasm.git@4671120cd8558ce62ee8672ebf3eb6f5216f909b",
  "third_party/externals/yasm/binaries"             : "https://chromium.googlesource.com/chromium/deps/yasm/binaries.git@52f9b3f4b0aa06da24ef8b123058bb61ee468881",

  "platform_tools/android/third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-5.1.0_r3",

  "platform_tools/chromeos/toolchain/src/third_party/chromite": "https://chromium.googlesource.com/chromiumos/chromite.git@d6a4c7e0ee4d53ddc5238dbddfc0417796a70e54",
  "platform_tools/chromeos/toolchain/src/third_party/pyelftools": "https://chromium.googlesource.com/chromiumos/third_party/pyelftools.git@bdc1d380acd88d4bfaf47265008091483b0d614e",

  # The line below is needed for compiling SkV8Example. Do not delete.
  #"third_party/externals/v8": "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",

  # sfntly is used by the PDF backend for font subsetting
  "third_party/externals/sfntly" : "https://chromium.googlesource.com/external/sfntly/cpp/src.git@1bdaae8fc788a5ac8936d68bf24f37d977a13dac",
  # ICU is needed for sfntly.
  "third_party/externals/icu" : "https://chromium.googlesource.com/chromium/deps/icu.git@ce41627e388fb46ab49671bd16a5db81dcd75a71",

  # sdl will be needed for native windows
  "third_party/externals/sdl" : "https://skia.googlesource.com/third_party/sdl@9b526d28cb2d7f0ccff0613c94bb52abc8f53b6f",
}

recursedeps = [ "common" ]
