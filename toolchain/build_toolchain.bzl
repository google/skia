"""
This file assembles a toolchain for Linux using the Clang Compiler glibc.

It downloads the necessary header files and pre-compiled static/shared libraries to
"""

# From https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz.sha256
clang_prefix = "clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04/"
clang_sha256 = "2c2fb857af97f41a5032e9ecadf7f78d3eff389a5cd3c9ec620d24f134ceb3c8"
clang_url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.0/clang+llvm-13.0.0-x86_64-linux-gnu-ubuntu-20.04.tar.xz"

# Files are expected to be in the mirror location named after their sha256 hash. The files should
# still have their file extension, as some of the Starlark functions sniff the file extension
# (e.g. download_and_extract). See //bazel/gcs_mirror for an automated way to update this mirror.
mirror_prefix = "https://storage.googleapis.com/skia-world-readable/bazel/"
# Set this to True to only use the files from the mirror host. This can be used to test the data
# in the mirrors is not corrupt and publicly accessible.
# If testing this, you need to delete the download cache, which defaults to
# ~/.cache/bazel/_bazel_$USER/cache/repos/v1/
# https://docs.bazel.build/versions/main/guide.html#the-repository-cache
force_test_of_mirrors = False

debs_to_install = [
    # These three comprise glibc. libc6 has the shared libraries, like libc itself, the math library
    # (libm), etc. linux-libc-dev has the header files specific to linux. libc6-dev has the libc
    # system headers (e.g. malloc.h, math.h).
    {
        # From https://packages.debian.org/bullseye/amd64/libc6/download
        "sha256": "3d9421c3fc0ef0d8ce57c0a149e1f8dbad78aba067f120be9e652af28902e346",
        "url": "https://ftp.debian.org/debian/pool/main/g/glibc/libc6_2.31-13+deb11u2_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/linux-libc-dev/download
        "sha256": "1bb053863873916cb8d5fa877cc4972a6279931783c1fd9e4339d0369a617af4",
        "url": "https://ftp.debian.org/debian/pool/main/l/linux/linux-libc-dev_5.10.84-1_amd64.deb",
    },
    {
        # From https://packages.debian.org/bullseye/amd64/libc6-dev/download
        "sha256": "1911bac1137f8f51359047d2fc94053f831abcfb50f1d7584e3ae95ea0831569",
        "url": "https://ftp.debian.org/debian/pool/main/g/glibc/libc6-dev_2.31-13+deb11u2_amd64.deb",
    },
    # These two put the X11 include files in ${PWD}/usr/include/X11
    # libx11-dev puts libX11.a in ${PWD}/usr/lib/x86_64-linux-gnu
    {
        # From https://packages.debian.org/bullseye/amd64/libx11-dev/download
        "sha256": "11e5f9dcded1a1226b3ee02847b86edce525240367b3989274a891a43dc49f5f",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libx11/libx11-dev_1.7.2-1_amd64.deb",
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
    # Xau is a dep of xcb
    {
        # From https://packages.debian.org/bullseye/amd64/libxau-dev/download
        "sha256": "d1a7f5d484e0879b3b2e8d512894744505e53d078712ce65903fef2ecfd824bb",
        "url": "https://ftp.debian.org/debian/pool/main/libx/libxau/libxau-dev_1.0.9-1_amd64.deb",
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
    # This provides libGL.so for us to link against.
    {
        # From https://packages.debian.org/bullseye/amd64/libgl1/download
        "sha256": "f300f9610b5f05f1ce566c4095f1bf2170e512ac5d201c40d895b8fce29dec98",
        "url": "https://ftp.debian.org/debian/pool/main/libg/libglvnd/libgl1_1.3.2-1_amd64.deb",
    },
]

def _download_and_extract_deb(ctx, deb, sha256, prefix, output = ""):
    """Downloads a debian file and extracts the data into the provided output directory"""

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download(
        url = _mirror([deb, mirror_prefix+sha256+".deb"]),
        output = "deb.ar",
        sha256 = sha256,
    )

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#execute
    # This uses the extracted llvm-ar that comes with clang.
    ctx.execute(["bin/llvm-ar", "x", "deb.ar"])

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#extract
    extract_info = ctx.extract(
        archive = "data.tar.xz",
        output = output,
        stripPrefix = prefix,
    )

    # Clean up
    ctx.delete("deb.ar")
    ctx.delete("data.tar.xz")
    ctx.delete("control.tar.xz")

def _build_cpp_toolchain_impl(ctx):
    # Download the clang toolchain (the extraction can take a while)
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download_and_extract
    download_info = ctx.download_and_extract(
        url = _mirror([clang_url, mirror_prefix+clang_sha256+".tar.xz"]),
        output = "",
        stripPrefix = clang_prefix,
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

    # Create a BUILD.bazel file that makes all the files in this subfolder
    # available for use in rules, i.e. in the toolchain declaration.
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#file
    ctx.file(
        "BUILD.bazel",
        content = """
filegroup(
    name = "all_files",
    srcs = glob([
        "**",
    ]),
    visibility = ["//visibility:public"]
)
""",
        executable = False,
    )

# If force_test_of_mirrors is set, return a list containing only the second item. This assumes
# that the given list will have a primary source and a mirror source (precisely two items).
def _mirror(arr):
    if force_test_of_mirrors:
        return [arr[1]]
    return arr

build_cpp_toolchain = repository_rule(
    implementation = _build_cpp_toolchain_impl,
    attrs = {},
    doc = "",
)
