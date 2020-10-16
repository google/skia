use_relative_paths = True

vars = {
  "checkout_chromium": False,
}

deps = {
  "buildtools"                            : "https://chromium.googlesource.com/chromium/buildtools.git@505de88083136eefd056e5ee4ca0f01fe9b33de8",
  "common"                                : "https://skia.googlesource.com/common.git@9737551d7a52c3db3262db5856e6bcd62c462b92",
  "third_party/externals/abseil-cpp"      : "https://chromium.googlesource.com/external/github.com/abseil/abseil-cpp.git@b56cbdd23834a65682c0b46f367f8679e83bc894",
  "third_party/externals/angle2"          : "https://chromium.googlesource.com/angle/angle.git@b4fb7cc9ec8f4c1751f84dcf5098b1ec5212a9ec",
  "third_party/externals/d3d12allocator"  : "https://skia.googlesource.com/external/github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator.git@169895d529dfce00390a20e69c2f516066fe7a3b",
  # Dawn requires jinja2 and markupsafe for the code generator, and glslang and shaderc for SPIRV compilation.
  # When the Dawn revision is updated these should be updated from the Dawn DEPS as well.
  "third_party/externals/dawn"            : "https://dawn.googlesource.com/dawn.git@b1938273e4050931cadf697a946a0bcb49821ce1",
  "third_party/externals/glslang"         : "https://chromium.googlesource.com/external/github.com/KhronosGroup/glslang@4dcc12d1a441b29d5901bc708bb1343d29d6459f",
  "third_party/externals/jinja2"          : "https://chromium.googlesource.com/chromium/src/third_party/jinja2@b41863e42637544c2941b574c7877d3e1f663e25",
  "third_party/externals/markupsafe"      : "https://chromium.googlesource.com/chromium/src/third_party/markupsafe@8f45f5cfa0009d2a70589bcda0349b8cb2b72783",
  "third_party/externals/shaderc"         : "https://chromium.googlesource.com/external/github.com/google/shaderc@011139094ec790ff7f32ea2d80286255fc9ed18b",
  "third_party/externals/dng_sdk"         : "https://android.googlesource.com/platform/external/dng_sdk.git@c8d0c9b1d16bfda56f15165d39e0ffa360a11123",
  "third_party/externals/egl-registry"    : "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry@a0bca08de07c7d7651047bedc0b653cfaaa4f2ae",
  "third_party/externals/expat"           : "https://chromium.googlesource.com/external/github.com/libexpat/libexpat.git@e976867fb57a0cd87e3b0fe05d59e0ed63c6febb",
  "third_party/externals/freetype"        : "https://skia.googlesource.com/third_party/freetype2.git@0a3d2bb99b45b72e1d45185ab054efa993d97210",
  "third_party/externals/harfbuzz"        : "https://chromium.googlesource.com/external/github.com/harfbuzz/harfbuzz.git@3a74ee528255cc027d84b204a87b5c25e47bff79",
  "third_party/externals/icu"             : "https://chromium.googlesource.com/chromium/deps/icu.git@dbd3825b31041d782c5b504c59dcfb5ac7dda08c",
  "third_party/externals/imgui"           : "https://skia.googlesource.com/external/github.com/ocornut/imgui.git@9418dcb69355558f70de260483424412c5ca2fce",
  "third_party/externals/libgifcodec"     : "https://skia.googlesource.com/libgifcodec@d06d2a6d42baf6c0c91cacc28df2542a911d05fe",
  "third_party/externals/libjpeg-turbo"   : "https://chromium.googlesource.com/chromium/deps/libjpeg_turbo.git@64fc43d52351ed52143208ce6a656c03db56462b",
  "third_party/externals/libpng"          : "https://skia.googlesource.com/third_party/libpng.git@386707c6d19b974ca2e3db7f5c61873813c6fe44",
  "third_party/externals/libwebp"         : "https://chromium.googlesource.com/webm/libwebp.git@55a080e50af655d1fbe0a5c22954835cdd59ff92",
  "third_party/externals/lua"             : "https://skia.googlesource.com/external/github.com/lua/lua.git@e354c6355e7f48e087678ec49e340ca0696725b1",
  "third_party/externals/microhttpd"      : "https://android.googlesource.com/platform/external/libmicrohttpd@748945ec6f1c67b7efc934ab0808e1d32f2fb98d",
  "third_party/externals/oboe"            : "https://chromium.googlesource.com/external/github.com/google/oboe.git@b02a12d1dd821118763debec6b83d00a8a0ee419",
  "third_party/externals/opencl-lib"      : "https://skia.googlesource.com/external/github.com/GPUOpen-Tools/common-lib-amd-APPSDK-3.0@4e6d30e406d2e5a65e1d65e404fe6df5f772a32b",
  "third_party/externals/opencl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenCL-Registry@932ed55c85f887041291cef8019e54280c033c35",
  "third_party/externals/opengl-registry" : "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry@14b80ebeab022b2c78f84a573f01028c96075553",
  "third_party/externals/piex"            : "https://android.googlesource.com/platform/external/piex.git@bb217acdca1cc0c16b704669dd6f91a1b509c406",
  "third_party/externals/sdl"             : "https://skia.googlesource.com/third_party/sdl@5d7cfcca344034aff9327f77fc181ae3754e7a90",
  "third_party/externals/sfntly"          : "https://chromium.googlesource.com/external/github.com/googlei18n/sfntly.git@b55ff303ea2f9e26702b514cf6a3196a2e3e2974",
  "third_party/externals/spirv-cross"     : "https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross@bdbef7b1f3982fe99a62d076043036abe6dd6d80",
  "third_party/externals/spirv-headers"   : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers.git@c0df742ec0b8178ad58c68cff3437ad4b6a06e26",
  "third_party/externals/spirv-tools"     : "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools.git@e95fbfb1f509ad7a7fdfb72ac35fe412d72fc4a4",
  "third_party/externals/swiftshader"     : "https://swiftshader.googlesource.com/SwiftShader@04bd4d8e4ece08cc4efbb4754665f46ec42eff79",
  #"third_party/externals/v8"              : "https://chromium.googlesource.com/v8/v8.git@5f1ae66d5634e43563b2d25ea652dfb94c31a3b4",
  "third_party/externals/wuffs"           : "https://skia.googlesource.com/external/github.com/google/wuffs.git@00cc8a50aa0c86b6bcb37e9ece8fb100047cc17b",
  "third_party/externals/zlib"            : "https://chromium.googlesource.com/chromium/src/third_party/zlib@eaf99a4e2009b0e5759e6070ad1760ac1dd75461",

  "../src": {
    "url": "https://chromium.googlesource.com/chromium/src.git@17855e1b657e0b88d7ae0f82282a726483283e9b",
    "condition": "checkout_chromium",
  },
}

recursedeps = [
  "common",
  "../src",
]

gclient_gn_args_from = 'src'
