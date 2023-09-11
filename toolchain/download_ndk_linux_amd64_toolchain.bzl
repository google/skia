"""
This file is copied from the SkCMS repository. Original file:
https://skia.googlesource.com/skcms/+/ba39d81f9797aa973bdf01aa6b0363b280352fba/toolchain/download_ndk_linux_amd64.bzl
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Path to the Android NDK from the point of view of the cc_toolchain rule.
# Note how this matches the name in http_archive.
NDK_PATH = "external/ndk_linux_amd64"

def download_ndk_linux_amd64_toolchain(name):
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
