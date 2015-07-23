use_relative_paths = True

# Dependencies on outside packages.
#
deps = {
  "common": "https://skia.googlesource.com/common.git@c282fe0b6e392b14f88d647cbd86e1a3ef5498e0",

  # There is some duplication here that might be worth cleaning up:
  #   - can use use our existing t_p/e/libjpeg instead of pulling it for Android?

  "third_party/externals/angle2"  : "https://chromium.googlesource.com/angle/angle.git@f9ac8fea307532e31cba6240801b4a56483591f8",
  "third_party/externals/freetype": "https://skia.googlesource.com/third_party/freetype2.git@VER-2-5-0-1",
  "third_party/externals/gyp"     : "https://chromium.googlesource.com/external/gyp.git@08429da7955a98ef764fafa223dd7de73f654b2d",
  "third_party/externals/harfbuzz": "https://skia.googlesource.com/third_party/harfbuzz.git@0.9.35",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/jsoncpp/jsoncpp.git@1afff032c83e26ddf7f2776e8b43de5ad666c1fa",
  "third_party/externals/libjpeg" : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@034e9a9747e0983bc19808ea70e469bc8342081f",
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@v0.4.3",
  "third_party/externals/nanomsg" : "https://skia.googlesource.com/third_party/nanomsg.git@0.4-beta",
  "third_party/externals/zlib"    : "https://chromium.googlesource.com/chromium/src/third_party/zlib@4ba7cdd0e7bf49d671645264f839838fc56e1492",
  # NOTE: If we update libpng, we may need to update the generated file at third_party/libpng/pnglibconf.h
  "third_party/externals/libpng"  : "https://skia.googlesource.com/third_party/libpng.git@070a616b8275277e18ef8ee91e2ca23f7bdc67d5",
  "third_party/externals/giflib"  : "https://android.googlesource.com/platform/external/giflib.git@android-5.1.0_r3",

  "third_party/externals/libjpeg-turbo"             : "https://skia.googlesource.com/third_party/libjpeg-turbo.git@trunk+fixvalgrind",
  # libjpeg-turbo depends on yasm to compile .asm files
  "third_party/externals/yasm/source/patched-yasm/" : "https://chromium.googlesource.com/chromium/deps/yasm/patched-yasm.git@4671120cd8558ce62ee8672ebf3eb6f5216f909b",
  "third_party/externals/yasm/binaries"             : "https://chromium.googlesource.com/chromium/deps/yasm/binaries.git@52f9b3f4b0aa06da24ef8b123058bb61ee468881",

  "platform_tools/android/third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-5.1.0_r3",
  "platform_tools/android/third_party/externals/jpeg" : "https://android.googlesource.com/platform/external/jpeg.git@android-5.1.0_r3",
  "platform_tools/android/third_party/externals/png" : "https://android.googlesource.com/platform/external/libpng.git@android-4.2.2_r1.2",

  "platform_tools/chromeos/toolchain/src/third_party/chromite": "https://chromium.googlesource.com/chromiumos/chromite.git@d6a4c7e0ee4d53ddc5238dbddfc0417796a70e54",
  "platform_tools/chromeos/toolchain/src/third_party/pyelftools": "https://chromium.googlesource.com/chromiumos/third_party/pyelftools.git@bdc1d380acd88d4bfaf47265008091483b0d614e",

  # The line below is needed for compiling SkV8Example. Do not delete.
  #"third_party/externals/v8": "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",

  # sfntly is used by the PDF backend for font subsetting
  "third_party/externals/sfntly" : "https://chromium.googlesource.com/external/sfntly/cpp/src.git@1bdaae8fc788a5ac8936d68bf24f37d977a13dac",
  # ICU is needed for sfntly.
  "third_party/externals/icu" : "https://chromium.googlesource.com/chromium/deps/icu.git@ce41627e388fb46ab49671bd16a5db81dcd75a71",
}

recursedeps = [ "common" ]
