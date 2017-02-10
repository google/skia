use_relative_paths = True

# Dependencies on outside packages.
#
deps = {
  "buildtools":  "https://chromium.googlesource.com/chromium/buildtools.git@55ad626b08ef971fd82a62b7abb325359542952b",

  "common": "https://skia.googlesource.com/common.git@9737551d7a52c3db3262db5856e6bcd62c462b92",

  # There is some duplication here that might be worth cleaning up:
  #   - can use use our existing t_p/e/libjpeg instead of pulling it for Android?

  "third_party/externals/angle2"  : "https://chromium.googlesource.com/angle/angle.git@57f17473791703ac82add77c3d77d13d8e2dfbc4",
  "third_party/externals/freetype": "https://skia.googlesource.com/third_party/freetype2.git@08fd250e1af0aa16d18012d39462e6ca9bbc6e90",
  "third_party/externals/harfbuzz": "https://skia.googlesource.com/third_party/harfbuzz.git@1.4.2",
  "third_party/externals/jsoncpp" : "https://chromium.googlesource.com/external/github.com/open-source-parsers/jsoncpp.git@1.0.0",
  "third_party/externals/libwebp" : "https://chromium.googlesource.com/webm/libwebp.git@v0.5.2-rc2",
  "third_party/externals/zlib"    : "https://chromium.googlesource.com/chromium/src/third_party/zlib@4576304a4b9835aa8646c9735b079e1d96858633",

  "third_party/externals/dng_sdk" : "https://android.googlesource.com/platform/external/dng_sdk.git@96443b262250c390b0caefbf3eed8463ba35ecae",
  "third_party/externals/piex"    : "https://android.googlesource.com/platform/external/piex.git@8f540f64b6c170a16fb7e6e52d61819705c1522a",

  "third_party/externals/libjpeg-turbo"             : "https://skia.googlesource.com/third_party/libjpeg-turbo.git@debddedc75850bcdeb8a57258572f48b802a4bb3",
  # libjpeg-turbo depends on yasm to compile .asm files
  "third_party/externals/yasm/source/patched-yasm/" : "https://chromium.googlesource.com/chromium/deps/yasm/patched-yasm.git@4671120cd8558ce62ee8672ebf3eb6f5216f909b",
  "third_party/externals/yasm/binaries"             : "https://chromium.googlesource.com/chromium/deps/yasm/binaries.git@52f9b3f4b0aa06da24ef8b123058bb61ee468881",

  "third_party/externals/expat" : "https://android.googlesource.com/platform/external/expat.git@android-6.0.1_r55",


  # The line below is needed for compiling SkV8Example. Do not delete.
  #"third_party/externals/v8": "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",

  # sfntly is used by the PDF backend for font subsetting
  "third_party/externals/sfntly" : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b18b09b6114b9b7fe6fc2f96d8b15e8a72f66916",
  # ICU is needed for sfntly.
  "third_party/externals/icu" : "https://chromium.googlesource.com/chromium/deps/icu.git@ec9c1133693148470ffe2e5e53576998e3650c1d",

  # sdl will be needed for native windows
  "third_party/externals/sdl" : "https://skia.googlesource.com/third_party/sdl@9b526d28cb2d7f0ccff0613c94bb52abc8f53b6f",

  # microhttpd for skiaserve
  "third_party/externals/microhttpd" : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",

  # imgui for Viewer/SampleApp widgets
  "third_party/externals/imgui" : "https://github.com/ocornut/imgui.git@6384eee34f08cb7eab8d835043e1738e4adcdf75",
}

recursedeps = [ "common" ]

hooks = []

import os
import sys
if os.path.exists('bin/fetch-gn'):
  hooks.append({ 'action': [sys.executable, 'bin/fetch-gn'] })
