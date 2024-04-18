"""
This file assembles a toolchain for an amd64 Linux host using the Clang Compiler and glibc.

It downloads the necessary headers, executables, and pre-compiled static/shared libraries to
the external subfolder of the Bazel cache (the same place third party deps are downloaded with
http_archive or similar functions in WORKSPACE.bazel). These will be able to be used via our
custom c++ toolchain configuration (see //toolchain/linux_amd64_toolchain_config.bzl)

Most files are downloaded as .deb files from packages.debian.org (with us acting as the dependency
resolver) and extracted to
  [outputRoot (aka Bazel cache)]/[outputUserRoot]/[outputBase]/external/clang_linux_amd64
  (See https://bazel.build/docs/output_directories#layout-diagram)
which will act as our sysroot.
"""

load(":clang_layering_check.bzl", "generate_system_module_map")
load(":utils.bzl", "gcs_mirror_only", "gcs_mirror_url")

# The clang from CIPD has no prefix, and we download it directly from our GCS bucket
# This is clang 15.0.1 and iwyu built from source.
# https://chrome-infra-packages.appspot.com/p/skia/bots/clang_linux/+/5h9JgVTkZk0fFuOyLUCHZXIFqG1b1TAdYG9fHTFLEzoC
clang_sha256 = "e61f498154e4664d1f16e3b22d4087657205a86d5bd5301d606f5f1d314b133a"

debs_to_install = [
    # These three comprise glibc. libc6 has the shared libraries, like libc itself, the math library
    # (libm), etc. linux-libc-dev has the header files specific to linux. libc6-dev has the libc
    # system headers (e.g. malloc.h, math.h).
    {
        # We use this old version of glibc because as of Nov 2022, many of our Swarming machines
        # are still on Debian 10. While many of the Bazel tasks can be run in RBE, using a newer
        # Debian 11 image (see //bazel/rbe/gce_linux_container/Dockerfile) some tasks need to be
        # run on these host machines using Debian 10. As a result, we need to compile and link
        # against a version of glibc that can be run on Debian 10 until we update those Swarming
        # hosts.
        # From https://packages.debian.org/buster/amd64/libc6/download
        "sha256": "980066e3e6124b8d84cdfd4cfa96d78a97cd659f8f3ba995bbcb887dad9ac237",
        "url": "https://security.debian.org/debian-security/pool/updates/main/g/glibc/libc6_2.28-10+deb10u2_amd64.deb",
    },
    {
        # From https://packages.debian.org/buster/amd64/linux-libc-dev/download
        "sha256": "e724656440d71d6316772fe58d7a8ac9634a0060a94af4e3b50e4f0a9e5a75e0",
        "url": "https://security.debian.org/debian-security/pool/updates/main/l/linux/linux-libc-dev_4.19.260-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/buster/amd64/libc6-dev/download
        "sha256": "6c11087f5bdc6a2a59fc6424e003dddede53fb97888ade2e35738448fa30a159",
        "url": "https://security.debian.org/debian-security/pool/updates/main/g/glibc/libc6-dev_2.28-10+deb10u2_amd64.deb",
    },
    # These two put the X11 include files in ${PWD}/usr/include/X11
    # libx11-dev puts libX11.a in ${PWD}/usr/lib/x86_64-linux-gnu
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-dev/download
        "sha256": "11e5f9dcded1a1226b3ee02847b86edce525240367b3989274a891a43dc49f5f",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libx11/libx11-dev_1.7.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-6/download
        "sha256": "086bd667fc07369472a923da015d182bb0c15a72228a5c0e6ddbcbeaab70acd2",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libx11/libx11-6_1.7.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/all/x11proto-dev/download
        "sha256": "d5568d587d9ad2664c34c14b0ac538ccb3c567e126ee5291085a8de704a565f5",
        "url": "https://ftp.debian.org/debian/pool/main/x/xorgproto/x11proto-dev_2020.1-1_all.deb",
    },
    # xcb is a dep of X11
    {
        # From https://packages.debian.org/bullseye/amd64/libxcb1-dev/download
        "sha256": "b75544f334c8963b8b7b0e8a88f8a7cde95a714dddbcda076d4beb669a961b58",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxcb/libxcb1-dev_1.14-3_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libxcb1/download
        "sha256": "d5e0f047ed766f45eb7473947b70f9e8fddbe45ef22ecfd92ab712c0671a93ac",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxcb/libxcb1_1.14-3_amd64.deb",
    },
    # Xau is a dep of xcb
    {
        # From https://packages.debian.org/bullseye/amd64/libxau-dev/download
        "sha256": "d1a7f5d484e0879b3b2e8d512894744505e53d078712ce65903fef2ecfd824bb",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxau/libxau-dev_1.0.9-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libxau6/download
        "sha256": "679db1c4579ec7c61079adeaae8528adeb2e4bf5465baa6c56233b995d714750",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxau/libxau6_1.0.9-1_amd64.deb",
    },

    # Xdmcp is a dep of xcb. libxdmcp-dev provides the the libXdmcp.so symlink (and the
    # .a if we want to statically include it). libxdmcp6 actually provides the .so file
    {
        # https://packages.debian.org/bullseye/amd64/libxdmcp-dev/download
        "sha256": "c6733e5f6463afd261998e408be6eb37f24ce0a64b63bed50a87ddb18ebc1699",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxdmcp/libxdmcp-dev_1.1.2-3_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/amd64/libxdmcp6/download
        "sha256": "ecb8536f5fb34543b55bb9dc5f5b14c9dbb4150a7bddb3f2287b7cab6e9d25ef",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxdmcp/libxdmcp6_1.1.2-3_amd64.deb",
    },
    # These two put GL include files in ${PWD}/usr/include/GL
    {
        # From https://packages.debian.org/bullseye/amd64/libgl-dev/download
        "sha256": "a6487873f2706bbabf9346cdb190f47f23a1464f31cecf92c363bac37c342f2f",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglvnd/libgl-dev_1.3.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libglx-dev/download
        "sha256": "5a50549948bc4363eab32b1083dad2165402c3628f2ee85e9a32563228cc61c1",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglvnd/libglx-dev_1.3.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libglx0/download
        "sha256": "cb642200f7e28e6dbb4075110a0b441880eeec35c8a00a2198c59c53309e5e17",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglvnd/libglx0_1.3.2-1_amd64.deb",
    },
    # This provides libGL.so for us to link against.
    {
        # From https://packages.debian.org/bullseye/amd64/libgl1/download
        "sha256": "f300f9610b5f05f1ce566c4095f1bf2170e512ac5d201c40d895b8fce29dec98",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglvnd/libgl1_1.3.2-1_amd64.deb",
    },
    # This is used by sk_app for Vulkan and Dawn on Unix.
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-xcb-dev/download
        "sha256": "80a2413ace2a0a073f2472059b9e589737cbf8a336fb6862684a5811bf640aa3",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libx11/libx11-xcb-dev_1.7.2-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-xcb1/download
        "sha256": "1f9f2dbe7744a2bb7f855d819f43167df095fe7d5291546bec12865aed045e0c",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libx11/libx11-xcb1_1.7.2-1_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/libfontconfig-dev/download
        "sha256": "7655d4238ee7e6ced13501006d20986cbf9ff08454a4e502d5aa399f83e28876",
        "url": "https://ftp.debian.org/debian/pool/main/f/fontconfig/libfontconfig-dev_2.13.1-4.2_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/amd64/libfontconfig1/download
        "sha256": "b92861827627a76e74d6f447a5577d039ef2f95da18af1f29aa98fb96baea4c1",
        "url": "https://ftp.debian.org/debian/pool/main/f/fontconfig/libfontconfig1_2.13.1-4.2_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/libglu1-mesa-dev/download
        "sha256": "5df6abeedb1f6986cec4b17810ef1a2773a5cd3291544abacc2bf602a9520893",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglu/libglu1-mesa-dev_9.0.1-1_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/amd64/libglu1-mesa/download
        "sha256": "479736c235af0537c1af8df4befc32e638a4e979961fdb02f366501298c50526",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglu/libglu1-mesa_9.0.1-1_amd64.deb",
    },
    # These are needed for rustc to link executables
    {
        # https://packages.debian.org/bullseye/amd64/libgcc-s1/download
        "sha256": "e478f2709d8474165bb664de42e16950c391f30eaa55bc9b3573281d83a29daf",
        "url": "https://ftp.debian.org/debian/pool/main/g/gcc-10/libgcc-s1_10.2.1-6_amd64.deb",
    },
    # needed for EGL support
    {
        # From https://packages.debian.org/bullseye/amd64/libegl-dev/download
        "sha256": "2847662b23487d5b1e467bca8cc8753baa880f794744a9b492c978bd5514b286",
        "url": "http://ftp.debian.org/debian/pool/main/libg/libglvnd/libegl-dev_1.3.2-1_amd64.deb",
    },
    {
        # https://packages.debian.org/bullseye/amd64/libgles-dev/download
        "sha256": "969e9197d8b8a36780f9b5d86f7c3066cdfef9dd7cdc3aee59a1870415c53578",
        "url": "http://ftp.debian.org/debian/pool/main/libg/libglvnd/libgles-dev_1.3.2-1_amd64.deb",
    },
]

def _download_and_extract_deb(ctx, deb, sha256, prefix, output = ""):
    """Downloads a debian file and extracts the data into the provided output directory"""

    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    # A .deb file has a data.tar.xz and a control.tar.xz, but the important contents
    # (i.e. the headers or libs) are in the data.tar.xz
    ctx.download_and_extract(
        url = gcs_mirror_url(deb, sha256),
        output = "tmp",
        sha256 = sha256,
    )

    # https://bazel.build/rules/lib/repository_ctx#extract
    ctx.extract(
        archive = "tmp/data.tar.xz",
        output = output,
        stripPrefix = prefix,
    )

    # Clean up
    ctx.delete("tmp")

def _download_linux_amd64_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://bazel.build/rules/lib/repository_ctx#download_and_extract
    ctx.download_and_extract(
        url = gcs_mirror_only(clang_sha256, ".zip"),
        output = "",
        sha256 = clang_sha256,
    )

    # Extract all the debs into our sysroot. This is very similar to installing them, except their
    # dependencies are not installed automatically.
    for deb in debs_to_install:
        _download_and_extract_deb(
            ctx,
            deb["url"],
            deb["sha256"],
            ".",
        )

    # Make -lgcc_s work
    ctx.symlink("lib/x86_64-linux-gnu/libgcc_s.so.1", "lib/x86_64-linux-gnu/libgcc_s.so")

    # This list of files lines up with _make_default_flags() in linux_amd64_toolchain_config.bzl
    # It is all locations that our toolchain could find a system header.
    builtin_include_directories = [
        "include/c++/v1",
        "include/x86_64-unknown-linux-gnu/c++/v1",
        "lib/clang/15.0.1/include",
        "usr/include",
        "usr/include/x86_64-linux-gnu",
    ]

    generate_system_module_map(
        ctx,
        module_file = "toolchain_system_headers.modulemap",
        folders = builtin_include_directories,
    )

    # Create a BUILD.bazel file that makes the files downloaded into the toolchain visible.
    # We have separate groups for each task because doing less work (sandboxing fewer files
    # or uploading less data to RBE) makes compiles go faster. We try to strike a balance
    # between minimal specifications and not having to edit this file often with our use
    # of globs.
    # https://bazel.build/rules/lib/repository_ctx#file
    ctx.file(
        "BUILD.bazel",
        content = """
# DO NOT EDIT THIS BAZEL FILE DIRECTLY
# Generated from ctx.file action in download_linux_amd64_toolchain.bzl
filegroup(
    name = "generated_module_map",
    srcs = ["toolchain_system_headers.modulemap"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "archive_files",
    srcs = [
        "bin/llvm-ar",
    ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "compile_files",
    srcs = [
        "bin/clang",
        "bin/include-what-you-use",
    ] + glob(
        include = [
            "include/c++/v1/**",
            "include/x86_64-unknown-linux-gnu/c++/v1/**",
            "usr/include/**",
            "lib/clang/15.0.1/**",
            "usr/include/x86_64-linux-gnu/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_files",
    srcs = [
        "bin/clang",
        "bin/ld.lld",
        "bin/lld",
        "lib/x86_64-unknown-linux-gnu/libc++.a",
        "lib/x86_64-unknown-linux-gnu/libc++abi.a",
        "lib/x86_64-unknown-linux-gnu/libunwind.a",
        "lib64/ld-linux-x86-64.so.2",
    ] + glob(
        include = [
            "lib/clang/15.0.1/lib/**",
            "lib/x86_64-linux-gnu/**",
            "usr/lib/x86_64-linux-gnu/**",
        ],
        allow_empty = False,
    ),
    visibility = ["//visibility:public"],
)

filegroup(
    name = "link_libs",
    srcs = [
        "lib/x86_64-unknown-linux-gnu/libc++.a",
        "lib/x86_64-unknown-linux-gnu/libc++abi.a",
],
    visibility = ["//visibility:public"],
)
""",
        executable = False,
    )

# https://bazel.build/rules/repository_rules
download_linux_amd64_toolchain = repository_rule(
    implementation = _download_linux_amd64_toolchain_impl,
    attrs = {},
    doc = "Downloads clang, and all supporting headers, executables, " +
          "and shared libraries required to build Skia on a Linux amd64 host",
)
