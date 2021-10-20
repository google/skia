"""
This file assembles a toolchain for Linux using the Clang Compiler and musl.

It currently makes use of musl and not glibc because the pre-compiled libraries from the latter
have absolute paths baked in and this makes linking difficult. The pre-compiled musl library
does not have this restriction and is much easier to statically link in as a result.

As inputs, it takes external URLs from which to download the clang binaries/libraries/includes
as well as the musl headers and pre-compiled binaries. Those files are downloaded and put
into one folder, with a little bit of re-arrangement so clang can find files (like the C runtime).
"""

def _download_and_extract_deb(ctx, deb, sha256, prefix, output = ""):
    """Downloads a debian file and extracts the data into the provided output directory"""

    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download(
        url = deb,
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
    # https://docs.bazel.build/versions/main/skylark/lib/repository_ctx.html#download
    download_info = ctx.download_and_extract(
        url = ctx.attr.clang_url,
        output = "",
        stripPrefix = ctx.attr.clang_prefix,
        sha256 = ctx.attr.clang_sha256,
    )

    # This puts the musl include files in ${PWD}/usr/include/x86_64-linux-musl
    # and the runtime files and lib.a files in ${PWD}/usr/lib/x86_64-linux-musl
    _download_and_extract_deb(
        ctx,
        ctx.attr.musl_dev_url,
        ctx.attr.musl_dev_sha256,
        ".",
    )

    # kjlubick@ cannot figure out how to get clang to look in ${PWD}/usr/lib/x86_64-linux-musl
    # for the crt1.o files, so we'll move them to ${PWD}/usr/lib/ where clang *is* looking.
    for file in ["crt1.o", "crtn.o", "Scrt1.o", "crti.o", "rcrt1.o"]:
        ctx.execute(["cp", "usr/lib/x86_64-linux-musl/" + file, "usr/lib/"])

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

build_cpp_toolchain = repository_rule(
    implementation = _build_cpp_toolchain_impl,
    attrs = {
        "clang_url": attr.string(mandatory = True),
        "clang_sha256": attr.string(mandatory = True),
        "clang_prefix": attr.string(mandatory = True),
        "musl_dev_url": attr.string(mandatory = True),
        "musl_dev_sha256": attr.string(mandatory = True),
    },
    doc = "",
)
