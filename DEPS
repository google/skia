use_relative_paths = True

# Dependencies on outside packages.
#
deps = {
  "common": "https://skia.googlesource.com/common.git@d7c2e2b9464e70e0f3847a330b930d008dc8c8db",

  # There is some duplication here that might be worth cleaning up:
  #   - both Android and ChromeOS pull the same giflib;
  #   - can use use our existing t_p/e/libjpeg instead of pulling it for Android?

  "third_party/externals/angle2"  : "https://chromium.googlesource.com/angle/angle.git@c415283b2bcd786e1a8c55c19ef3511eb2b3928c",
  "third_party/externals/freetype": "https://skia.googlesource.com/third_party/freetype2.git@VER-2-5-0-1",
  "third_party/externals/gyp"     : "https://chromium.googlesource.com/external/gyp.git@dd831fd86e7a254c696f53944333562466e453ad",
  "third_party/externals/harfbuzz": "https://skia.googlesource.com/third_party/harfbuzz.git@0.9.35",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/jsoncpp/jsoncpp.git@1afff032c83e26ddf7f2776e8b43de5ad666c1fa",
  "third_party/externals/libjpeg" : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@82ce8a6d4ebe12a177c0c3597192f2b4f09e81c3",
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@3fe91635df8734b23f3c1b9d1f0c4fa8cfaf4e39",
  "third_party/externals/nanomsg" : "https://skia.googlesource.com/third_party/nanomsg.git@0.4-beta",
  "third_party/externals/zlib"    : "https://chromium.googlesource.com/chromium/src/third_party/zlib@4ba7cdd0e7bf49d671645264f839838fc56e1492",
  # NOTE: If we update libpng, we may need to update the generated file at third_party/libpng/pnglibconf.h
  "third_party/externals/libpng"  : "git://git.code.sf.net/p/libpng/code@070a616b8275277e18ef8ee91e2ca23f7bdc67d5",

  "platform_tools/android/third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-4.2.2_r1.2",
  "platform_tools/android/third_party/externals/gif" : "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
  "platform_tools/android/third_party/externals/jpeg" : "https://android.googlesource.com/platform/external/jpeg.git@ef1b83013e7814622a9d11579878d342e84580b7",
  "platform_tools/android/third_party/externals/png" : "https://android.googlesource.com/platform/external/libpng.git@android-4.2.2_r1.2",

  "platform_tools/chromeos/third_party/externals/gif" : "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
  "platform_tools/chromeos/toolchain/src/third_party/chromite": "https://chromium.googlesource.com/chromiumos/chromite.git@d6a4c7e0ee4d53ddc5238dbddfc0417796a70e54",
  "platform_tools/chromeos/toolchain/src/third_party/pyelftools": "https://chromium.googlesource.com/chromiumos/third_party/pyelftools.git@bdc1d380acd88d4bfaf47265008091483b0d614e",

  # The line below is needed for compiling SkV8Example. Do not delete.
  #"third_party/externals/v8": "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
}

recursedeps = [ "common" ]
