"""
This file exports the various toolchains for the hosts that we support building Skia on.

Supported:
 - Linux amd64
 - Mac (one toolchain for both M1 and Intel CPUs)

Planned:
 - Windows amd64

"""

load("//bazel:cipd_install.bzl", "cipd_install")
load(":download_linux_amd64_toolchain.bzl", "download_linux_amd64_toolchain")
load(":download_mac_toolchain.bzl", "download_mac_toolchain")
load(":download_ndk_linux_amd64_toolchain.bzl", "download_ndk_linux_amd64_toolchain")
load(":download_windows_amd64_toolchain.bzl", "download_windows_amd64_toolchain")

# This key in this dictionary (and thus the name passed into the rule) controls what the subfolder
# will be called in the external directory. It must match what we use in the appropriate
# toolchain_config.bzl file or it will not be able to locate the sysroot to build with.
name_toolchain = {
    "clang_linux_amd64": download_linux_amd64_toolchain,
    "clang_mac": download_mac_toolchain,
    "clang_windows_amd64": download_windows_amd64_toolchain,
    "ndk_linux_amd64": download_ndk_linux_amd64_toolchain,
}

def download_toolchains_for_skia(*args):
    """
    Point Bazel to the correct rules for downloading the different toolchains.

    Args:
        *args: multiple toolchains, see top of file for
               list of supported toolchains.
    """

    for toolchain_name in args:
        if toolchain_name not in name_toolchain:
            fail("unrecognized toolchain name " + toolchain_name)
        download_toolchain = name_toolchain[toolchain_name]
        download_toolchain(name = toolchain_name)

    cipd_install(
        name = "win_toolchain",
        build_file = "//bazel/external/win_toolchain:BUILD.bazel",
        cipd_package = "skia/bots/win_toolchain",
        postinstall_cmds_posix = [
            # This toolchain only works on Windows, but because of the way our Bazel files are
            # structured, we need to create vars.bzl on posix platforms as well. We mimic all of the
            # variables it should contain with placeholder values.
            "echo \"MSVC_VERSION = 'this-toolchain-only-works-on-windows'\" > vars.bzl",
            "echo \"MSVC_INCLUDE = 'this-toolchain-only-works-on-windows'\" >> vars.bzl",
            "echo \"MSVC_LIB = 'this-toolchain-only-works-on-windows'\" >> vars.bzl",
            "echo \"WIN_SDK_VERSION = 'this-toolchain-only-works-on-windows'\" >> vars.bzl",
            "echo \"WIN_SDK_INCLUDE = 'this-toolchain-only-works-on-windows'\" >> vars.bzl",
            "echo \"WIN_SDK_LIB = 'this-toolchain-only-works-on-windows'\" >> vars.bzl",
        ],
        postinstall_cmds_win = [
            # Both MSVC and the Windows SDK have version numbers as part of the include and library
            # paths. In order to avoid hard-coding those version numbers, we find them after
            # downloading the package and dynamically create @win_toolchain//vars.bzl to export them.
            #
            # Note: This just uses a simple string sort to find the highest-numbered version present
            #       in the directory. This could cause incorrect results, for example "10.0" will sort
            #       before "9.0" alphabetically, even though 10 is greater than 9. In practice, we only
            #       expect to see a single version in these directories.
            #
            # Note: This is ugly because Bazel runs these commands in individual shells, so we can't
            #       store intermediate state in variables.
            "\"MSVC_VERSION = '$((Get-ChildItem -Path VC/Tools/MSVC | sort Name | select -Last 1).BaseName)'\" | Out-File -encoding ASCII -FilePath vars.bzl",
            "\"MSVC_INCLUDE = 'VC/Tools/MSVC/' + MSVC_VERSION + '/include'\" | Out-File -encoding ASCII -FilePath vars.bzl -Append",
            "\"MSVC_LIB = 'VC/Tools/MSVC/' + MSVC_VERSION + '/lib'\" | Out-File -encoding ASCII -FilePath vars.bzl -Append",
            "\"WIN_SDK_VERSION = '$((Get-ChildItem -Path win_sdk/Include | sort Name | select -Last 1).BaseName)'\" | Out-File -encoding ASCII -FilePath vars.bzl -Append",
            "\"WIN_SDK_INCLUDE = 'win_sdk/Include/' + WIN_SDK_VERSION\" | Out-File -encoding ASCII -FilePath vars.bzl -Append",
            "\"WIN_SDK_LIB = 'win_sdk/Lib/' + WIN_SDK_VERSION\" | Out-File -encoding ASCII -FilePath vars.bzl -Append",
        ],
        # From https://chrome-infra-packages.appspot.com/p/skia/bots/win_toolchain/+/89-wQAUmYRJEcNlZ-vg1Cu9MLamzlq0ZGiySLXoBxD8C
        sha256 = "f3dfb040052661124470d959faf8350aef4c2da9b396ad191a2c922d7a01c43f",
        tag = "version:15",
    )
