"""
This file is copied from the SkCMS repository. Original file:
https://skia.googlesource.com/skcms/+/ba39d81f9797aa973bdf01aa6b0363b280352fba/toolchain/ndk_linux_arm64_toolchain_config.bzl

This module defines the ndk_cc_toolchain_config rule.

This file is based on the `external/androidndk/cc_toolchain_config.bzl` file produced by the
built-in `android_ndk_repository` Bazel rule[1], which was used to build the SkCMS repository up
until this revision[2].

The paths in this file point to locations inside the expanded Android NDK ZIP file (found at
external/ndk_linux_amd64), and must be updated every time we upgrade to a new Android NDK version.

[1] https://github.com/bazelbuild/bazel/blob/4710ef82ce34572878e07c52e83a0144d707f140/src/main/java/com/google/devtools/build/lib/bazel/rules/android/AndroidNdkRepositoryFunction.java#L422
[2] https://skia.googlesource.com/skcms/+/30c8e303800c256febb03a09fdcda7f75d119b1b/WORKSPACE#22
"""

load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
    "with_feature_set",
)
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load(":download_ndk_linux_amd64_toolchain.bzl", "NDK_PATH")

# Supported CPUs.
_ARMEABI_V7A = "armeabi-v7a"
_ARM64_V8A = "arm64-v8a"

_all_compile_actions = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.linkstamp_compile,
    ACTION_NAMES.assemble,
    ACTION_NAMES.preprocess_assemble,
    ACTION_NAMES.cpp_header_parsing,
    ACTION_NAMES.cpp_module_compile,
    ACTION_NAMES.cpp_module_codegen,
    ACTION_NAMES.clif_match,
    ACTION_NAMES.lto_backend,
]

_all_link_actions = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _get_default_compile_flags(cpu):
    if cpu == _ARMEABI_V7A:
        return [
            "-D__ANDROID_API__=29",
            "-isystem",
            NDK_PATH + "/sysroot/usr/include/arm-linux-androideabi",
            "-target",
            "armv7-none-linux-androideabi",
            "-march=armv7-a",
            "-mfloat-abi=softfp",
            "-mfpu=vfpv3-d16",
            "-gcc-toolchain",
            NDK_PATH + "/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64",
            "-fpic",
            "-no-canonical-prefixes",
            "-Wno-invalid-command-line-argument",
            "-Wno-unused-command-line-argument",
            "-funwind-tables",
            "-fstack-protector-strong",
            "-fno-addrsig",
        ]
    if cpu == _ARM64_V8A:
        return [
            "-gcc-toolchain",
            NDK_PATH + "/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64",
            "-target",
            "aarch64-none-linux-android",
            "-fpic",
            "-isystem",
            NDK_PATH + "/sysroot/usr/include/aarch64-linux-android",
            "-D__ANDROID_API__=29",
            "-no-canonical-prefixes",
            "-Wno-invalid-command-line-argument",
            "-Wno-unused-command-line-argument",
            "-funwind-tables",
            "-fstack-protector-strong",
            "-fno-addrsig",
        ]
    fail("Unknown CPU: " + cpu)

def _get_default_link_flags(cpu):
    if cpu == _ARMEABI_V7A:
        return [
            "-target",
            "armv7-none-linux-androideabi",
            "-gcc-toolchain",
            NDK_PATH + "/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64",
            "-L",
            NDK_PATH + "/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
            "-no-canonical-prefixes",
            "-Wl,-z,relro",
            "-lm",
        ]
    if cpu == _ARM64_V8A:
        return [
            "-gcc-toolchain",
            NDK_PATH + "/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64",
            "-target",
            "aarch64-none-linux-android",
            "-L",
            NDK_PATH + "/sources/cxx-stl/llvm-libc++/libs/arm64-v8a",
            "-no-canonical-prefixes",
            "-Wl,-z,relro",
            "-lm",
        ]
    fail("Unknown CPU: " + cpu)

def _get_default_dbg_flags(cpu):
    if cpu == _ARMEABI_V7A:
        return ["-g", "-fno-strict-aliasing", "-O0", "-UNDEBUG"]
    if cpu == _ARM64_V8A:
        return ["-O0", "-g", "-UNDEBUG"]
    fail("Unknown CPU: " + cpu)

def _get_default_opt_flags(cpu):
    if cpu == _ARMEABI_V7A:
        return ["-mthumb", "-Os", "-g", "-DNDEBUG"]
    if cpu == _ARM64_V8A:
        return ["-O2", "-g", "-DNDEBUG"]
    fail("Unknown CPU: " + cpu)

def _get_toolchain_identifier(cpu):
    if cpu == _ARMEABI_V7A:
        return "ndk-armeabi-v7a-toolchain"
    if cpu == _ARM64_V8A:
        return "ndk-arm64-v8a-toolchain"
    fail("Unknown CPU: " + cpu)

def _get_target_system_name(cpu):
    if cpu == _ARMEABI_V7A:
        return "arm-linux-androideabi"
    if cpu == _ARM64_V8A:
        return "aarch64-linux-android"
    fail("Unknown CPU: " + cpu)

def _get_builtin_sysroot(cpu):
    if cpu == _ARMEABI_V7A:
        return NDK_PATH + "/platforms/android-29/arch-arm"
    if cpu == _ARM64_V8A:
        return NDK_PATH + "/platforms/android-29/arch-arm64"
    fail("Unknown CPU: " + cpu)

def _get_tool_paths(cpu):
    # The cc_common.create_cc_toolchain_config_info function expects tool paths to point to files
    # under the directory in which it is invoked. This means we cannot directly reference tools
    # under external/android_ndk. The solution is to use "trampoline" scripts that pass through
    # any command-line arguments to the NDK binaries under external/android_sdk.

    if cpu == _ARMEABI_V7A:
        return [
            tool_path(
                name = "ar",
                path = "android_trampolines/arm-linux-androideabi-ar.sh",
            ),
            tool_path(
                name = "cpp",
                path = "android_trampolines/clang.sh",
            ),
            tool_path(
                name = "dwp",
                path = "android_trampolines/arm-linux-androideabi-dwp.sh",
            ),
            tool_path(
                name = "gcc",
                path = "android_trampolines/clang.sh",
            ),
            tool_path(
                name = "gcov",
                path = "/bin/false",
            ),
            tool_path(
                name = "ld",
                path = "android_trampolines/arm-linux-androideabi-ld.sh",
            ),
            tool_path(
                name = "nm",
                path = "android_trampolines/arm-linux-androideabi-nm.sh",
            ),
            tool_path(
                name = "objcopy",
                path = "android_trampolines/arm-linux-androideabi-objcopy.sh",
            ),
            tool_path(
                name = "objdump",
                path = "android_trampolines/arm-linux-androideabi-objdump.sh",
            ),
            tool_path(
                name = "strip",
                path = "android_trampolines/arm-linux-androideabi-strip.sh",
            ),
        ]
    if cpu == _ARM64_V8A:
        return [
            tool_path(
                name = "ar",
                path = "android_trampolines/aarch64-linux-android-ar.sh",
            ),
            tool_path(
                name = "cpp",
                path = "android_trampolines/clang.sh",
            ),
            tool_path(
                name = "dwp",
                path = "android_trampolines/aarch64-linux-android-dwp.sh",
            ),
            tool_path(
                name = "gcc",
                path = "android_trampolines/clang.sh",
            ),
            tool_path(
                name = "gcov",
                path = "/bin/false",
            ),
            tool_path(
                name = "ld",
                path = "android_trampolines/aarch64-linux-android-ld.sh",
            ),
            tool_path(
                name = "nm",
                path = "android_trampolines/aarch64-linux-android-nm.sh",
            ),
            tool_path(
                name = "objcopy",
                path = "android_trampolines/aarch64-linux-android-objcopy.sh",
            ),
            tool_path(
                name = "objdump",
                path = "android_trampolines/aarch64-linux-android-objdump.sh",
            ),
            tool_path(
                name = "strip",
                path = "android_trampolines/aarch64-linux-android-strip.sh",
            ),
        ]
    fail("Unknown CPU: " + cpu)

def _ndk_cc_toolchain_config_impl(ctx):
    default_compile_flags = _get_default_compile_flags(ctx.attr.cpu)
    unfiltered_compile_flags = [
        "-isystem",
        NDK_PATH + "/sources/cxx-stl/llvm-libc++/include",
        "-isystem",
        NDK_PATH + "/sources/cxx-stl/llvm-libc++abi/include",
        "-isystem",
        NDK_PATH + "/sources/android/support/include",
        "-isystem",
        NDK_PATH + "/sysroot/usr/include",
    ]
    default_link_flags = _get_default_link_flags(ctx.attr.cpu)
    default_fastbuild_flags = [""]
    default_dbg_flags = _get_default_dbg_flags(ctx.attr.cpu)
    default_opt_flags = _get_default_opt_flags(ctx.attr.cpu)

    opt_feature = feature(name = "opt")
    fastbuild_feature = feature(name = "fastbuild")
    dbg_feature = feature(name = "dbg")
    supports_dynamic_linker_feature = feature(name = "supports_dynamic_linker", enabled = True)
    supports_pic_feature = feature(name = "supports_pic", enabled = True)
    static_link_cpp_runtimes_feature = feature(name = "static_link_cpp_runtimes", enabled = True)

    default_compile_flags_feature = feature(
        name = "default_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [flag_group(flags = default_compile_flags)],
            ),
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [flag_group(flags = default_fastbuild_flags)],
                with_features = [with_feature_set(features = ["fastbuild"])],
            ),
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [flag_group(flags = default_dbg_flags)],
                with_features = [with_feature_set(features = ["dbg"])],
            ),
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [flag_group(flags = default_opt_flags)],
                with_features = [with_feature_set(features = ["opt"])],
            ),
            flag_set(
                actions = [ACTION_NAMES.cpp_compile],
                # TODO(kjlubick) update this when updating the NDK
                flag_groups = [flag_group(flags = ["-std=c++2a"])],
            ),
        ],
    )

    default_link_flags_feature = feature(
        name = "default_link_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = _all_link_actions,
                flag_groups = [flag_group(flags = default_link_flags)],
            ),
        ],
    )

    user_compile_flags_feature = feature(
        name = "user_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = ["%{user_compile_flags}"],
                        iterate_over = "user_compile_flags",
                        expand_if_available = "user_compile_flags",
                    ),
                ],
            ),
        ],
    )

    sysroot_feature = feature(
        name = "sysroot",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = _all_compile_actions + _all_link_actions,
                flag_groups = [
                    flag_group(
                        flags = ["--sysroot=%{sysroot}"],
                        expand_if_available = "sysroot",
                    ),
                ],
            ),
        ],
    )

    unfiltered_compile_flags_feature = feature(
        name = "unfiltered_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = _all_compile_actions,
                flag_groups = [flag_group(flags = unfiltered_compile_flags)],
            ),
        ],
    )

    features = [
        default_compile_flags_feature,
        default_link_flags_feature,
        supports_dynamic_linker_feature,
        supports_pic_feature,
        static_link_cpp_runtimes_feature,
        fastbuild_feature,
        dbg_feature,
        opt_feature,
        user_compile_flags_feature,
        sysroot_feature,
        unfiltered_compile_flags_feature,
    ]

    cxx_builtin_include_directories = [
        NDK_PATH + "/toolchains/llvm/prebuilt/linux-x86_64/lib64/clang/9.0.9/include",
        "%sysroot%/usr/include",
        NDK_PATH + "/sysroot/usr/include",
    ]

    # https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = _get_toolchain_identifier(ctx.attr.cpu),
        host_system_name = "local",
        target_system_name = _get_target_system_name(ctx.attr.cpu),
        target_cpu = ctx.attr.cpu,
        target_libc = "local",
        compiler = "clang9.0.9",
        abi_version = ctx.attr.cpu,
        abi_libc_version = "local",
        features = features,
        tool_paths = _get_tool_paths(ctx.attr.cpu),
        cxx_builtin_include_directories = cxx_builtin_include_directories,
        builtin_sysroot = _get_builtin_sysroot(ctx.attr.cpu),
    )

ndk_cc_toolchain_config = rule(
    implementation = _ndk_cc_toolchain_config_impl,
    attrs = {
        "cpu": attr.string(
            mandatory = True,
            values = [_ARMEABI_V7A, _ARM64_V8A],
            doc = "Target CPU.",
        ),
    },
    provides = [CcToolchainConfigInfo],
)
