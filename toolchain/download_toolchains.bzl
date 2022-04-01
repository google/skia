"""
This file exports the various toolchains for the hosts that we support building Skia on.

Supported:
 - Linux amd64

Planned:
 - Mac M1
 - Windows amd64

"""

load(":download_linux_amd64_toolchain.bzl", "download_linux_amd64_toolchain")
load(":download_mac_m1_toolchain.bzl", "download_mac_m1_toolchain")

# This key in this dictionary (and thus the name passed into the rule) controls what the subfolder
# will be called in the external directory. It must match what we use in the appropriate
# toolchain_config.bzl file or it will not be able to locate the sysroot to build with.
name_toolchain = {
    "clang_linux_amd64": download_linux_amd64_toolchain,
    "clang_mac_m1": download_mac_m1_toolchain,
}

def download_toolchains_for_skia(*args):
    for toolchain_name in args:
        if toolchain_name not in name_toolchain:
            fail("unrecognized toolchain name " + toolchain_name)
        download_toolchain = name_toolchain[toolchain_name]
        download_toolchain(name = toolchain_name)
