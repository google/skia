# This file is copied from the SkCMS repository. Original file:
# https://skia.googlesource.com/skcms/+/ba39d81f9797aa973bdf01aa6b0363b280352fba/toolchain/ndk.BUILD

# This file is based on the `external/androidndk/BUILD.bazel` file produced by the built-in
# `android_ndk_repository` Bazel rule[1], which was used to build the SkCMS repository up until
# this revision[2].
#
# The paths in this file point to locations inside the expanded Android NDK ZIP file (found at
# external/ndk_linux_amd64), and must be updated every time we upgrade to a new Android NDK version.
#
# [1] https://github.com/bazelbuild/bazel/blob/4710ef82ce34572878e07c52e83a0144d707f140/src/main/java/com/google/devtools/build/lib/bazel/rules/android/AndroidNdkRepositoryFunction.java
# [2] https://skia.googlesource.com/skcms/+/30c8e303800c256febb03a09fdcda7f75d119b1b/WORKSPACE#22

filegroup(
    name = "arm64_v8a_all_files",
    srcs = glob(["toolchains/llvm/**"]) + glob([
        "platforms/android-29/arch-arm64/**/*",
        "sources/cxx-stl/llvm-libc++/include/**/*",
        "sources/cxx-stl/llvm-libc++abi/include/**/*",
        "sources/android/support/include/**/*",
        "sysroot/**/*",
        "toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/**/*",
    ]) + [
        ":arm64_v8a_dynamic_runtime_libraries",
        ":arm64_v8a_static_runtime_libraries",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "arm64_v8a_dynamic_runtime_libraries",
    srcs = glob(["sources/cxx-stl/llvm-libc++/libs/arm64-v8a/*.so"]),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "arm64_v8a_static_runtime_libraries",
    srcs = glob(["sources/cxx-stl/llvm-libc++/libs/arm64-v8a/*.a"]),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "armeabi_v7a_all_files",
    srcs = glob(["toolchains/llvm/**"]) + glob([
        "platforms/android-29/arch-arm/**/*",
        "sources/cxx-stl/llvm-libc++/include/**/*",
        "sources/cxx-stl/llvm-libc++abi/include/**/*",
        "sources/android/support/include/**/*",
        "sysroot/**/*",
        "toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/**/*",
    ]) + [
        ":armeabi_v7a_dynamic_runtime_libraries",
        ":armeabi_v7a_static_runtime_libraries",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "armeabi_v7a_dynamic_runtime_libraries",
    srcs = glob(["sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/*.so"]),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "armeabi_v7a_static_runtime_libraries",
    srcs = glob(["sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/*.a"]),
    visibility = ["//visibility:public"],
)
