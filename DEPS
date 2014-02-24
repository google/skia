use_relative_paths = True

# Dependencies on outside packages, as needed for developers/bots that use
# "gclient" instead of raw SVN access.
#
# For now, this must be maintained in parallel with "SVN externals"
# dependencies for developers who use raw SVN instead of "gclient".
# See third_party/externals/README
#
deps = {
  # DEPS using https://chromium.googlesource.com are pulled from chromium @ r205199
  # (see https://chromium.googlesource.com/chromium/chromium/+/c59bfa8ef877f45bfa859669053859857af1d279)
  "third_party/externals/angle" : "https://chromium.googlesource.com/external/angleproject.git",
  "third_party/externals/fontconfig" : "https://skia.googlesource.com/third_party/fontconfig.git@2.11.0",
  "third_party/externals/freetype" : "https://skia.googlesource.com/third_party/freetype2.git@VER-2-5-0-1",
  "third_party/externals/gyp" : "https://chromium.googlesource.com/external/gyp.git@5917c6a6b77c9e97a0cbb66847194381bd36ec4c",
  "third_party/externals/iconv" : "https://skia.googlesource.com/third_party/libiconv.git@v1.14",
  "third_party/externals/libjpeg" : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@82ce8a6d4ebe12a177c0c3597192f2b4f09e81c3",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/jsoncpp/jsoncpp.git@ab1e40f3bce061ea6f9bdc60351d6cde2a4f872b",
  # Move to a webp version newer than 0.3.0 to pickup NEON fix for iOS
  "third_party/externals/jsoncpp-chromium" : "https://chromium.googlesource.com/chromium/src/third_party/jsoncpp.git@41239939c0c60481f34887d52c038facf05f5533",
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@3fe91635df8734b23f3c1b9d1f0c4fa8cfaf4e39",
  "third_party/externals/poppler" : "https://skia.googlesource.com/third_party/poppler.git@poppler-0.22.5",
  # "third_party/externals/v8" : "git://github.com/v8/v8.git@d15b0f0f2099dbd72867f3df70e9aaf5b8afbd2c",
}

deps_os = {
  "android": {
    "platform_tools/android/third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/gif" : "https://android.googlesource.com/platform/external/giflib.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/png" : "https://android.googlesource.com/platform/external/libpng.git@android-4.2.2_r1.2",
    "platform_tools/android/third_party/externals/jpeg" : "https://android.googlesource.com/platform/external/jpeg.git@android-4.2.2_r1.2",
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
