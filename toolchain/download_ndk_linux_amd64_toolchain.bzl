load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Path to the Android NDK from the point of view of the cc_toolchain rule.
# This name is because of how the module use_repo prefixes extensions
NDK_PATH = "external/+download_ndk_linux_amd64_toolchain+ndk_linux_amd64"

def _download_ndk_linux_amd64_toolchain_impl(ctx):
    """Downloads the Android NDK under external/ndk_linux_amd64."""

    # Archive taken from https://github.com/android/ndk/wiki/Unsupported-Downloads#r21e.
    http_archive(
        name = "ndk_linux_amd64",
        urls = [
            "https://dl.google.com/android/repository/android-ndk-r21e-linux-x86_64.zip",
            "https://storage.googleapis.com/skia-world-readable/bazel/ad7ce5467e18d40050dc51b8e7affc3e635c85bd8c59be62de32352328ed467e.zip",
        ],
        sha256 = "ad7ce5467e18d40050dc51b8e7affc3e635c85bd8c59be62de32352328ed467e",
        strip_prefix = "android-ndk-r21e",
        build_file = Label("//toolchain:ndk.BUILD"),
    )

download_ndk_linux_amd64_toolchain = module_extension(
    implementation = _download_ndk_linux_amd64_toolchain_impl,
)
