use_relative_paths = True

# Dependencies on outside packages.
#
deps = {
  "common": "https://skia.googlesource.com/common.git@5dd2918f96b277ebe04eb66469ec3d5c7ba6952a",

  "third_party/externals/angle2" : "https://chromium.googlesource.com/angle/angle.git@23a8a433529d9db23882c702a29d5e594841563d",
  "third_party/externals/freetype" : "https://skia.googlesource.com/third_party/freetype2.git@VER-2-5-0-1",
  "third_party/externals/gyp" : "https://chromium.googlesource.com/external/gyp.git@3917682a16d5c19ff3576a8be0ffdb3a332954b1",
  "third_party/externals/libjpeg" : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@82ce8a6d4ebe12a177c0c3597192f2b4f09e81c3",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/jsoncpp/jsoncpp.git@1afff032c83e26ddf7f2776e8b43de5ad666c1fa",
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@3fe91635df8734b23f3c1b9d1f0c4fa8cfaf4e39",
  "third_party/externals/nanomsg": "git://github.com/nanomsg/nanomsg.git@0.4-beta",
}

recursedeps = [
  "common",
]

deps_os = {
  "android": {
    "platform_tools/android/third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/gif" : "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/png" : "https://android.googlesource.com/platform/external/libpng.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/jpeg" :
      "https://android.googlesource.com/platform/external/jpeg.git@ef1b83013e7814622a9d11579878d342e84580b7",
  },
  "chromeos": {
    "platform_tools/chromeos/third_party/externals/gif" : "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
    "platform_tools/chromeos/toolchain/src/third_party/chromite": "https://chromium.googlesource.com/chromiumos/chromite.git@d6a4c7e0ee4d53ddc5238dbddfc0417796a70e54",
    "platform_tools/chromeos/toolchain/src/third_party/pyelftools": "https://chromium.googlesource.com/chromiumos/third_party/pyelftools.git@bdc1d380acd88d4bfaf47265008091483b0d614e",
  },

  # barelinux is a DEPS target that has no shared libraries to link
  # to, similar to android or chromeos.
  "barelinux": {
    "third_party/externals/giflib" :
      "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
    "third_party/externals/libpng" :
       "https://android.googlesource.com/platform/external/libpng.git@android-4.2.2_r1.2",
    "third_party/externals/zlib" :
       "https://android.googlesource.com/platform/external/zlib.git@android-4.2.2_r1.2",
  }
}

#hooks = [
#  {
#    # A change to a .gyp, .gypi, or to GYP itself should run the generator.
#    "pattern": ".",
#    "action": ["python", "trunk/gyp_skia"],
#  },
#]
