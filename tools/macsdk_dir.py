#!/usr/bin/env python3
#
# Copyright 2023 Google LLC.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script writes the full path to the MacSDK that is being used
by the clang_mac toolchain for builds within this workspace. This
path is created by //toolchain/download_mac_toolchain.bzl when
downloading the mac toolchain, and the MacSDK directory is populated
with symlinks to XCode's MacSDK contents.
"""

import codecs
import hashlib
import os
import subprocess
import sys
from pathlib import Path


def GetWorkspaceDir() -> str:
    """Return the workspace directory containing this script."""
    this_script_path = Path(os.path.realpath(__file__))
    return str(this_script_path.parent.parent)


def GetBazelWorkspaceHash() -> str:
    """Return the Bazel hash for this workspace.

    This is the MD5 has of the full path to the workspace. See
    https://bazel.build/remote/output-directories#layout-diagram for more detail."""
    ws = GetWorkspaceDir().encode("utf-8")
    return hashlib.md5(ws).hexdigest()


def GetBazelRepositoryCacheDir() -> str:
    """Return the Bazel repository cache directory."""

    prev_cwd = os.getcwd()
    os.chdir(GetWorkspaceDir())
    cmd = ["bazelisk", "info", "repository_cache"]
    output = subprocess.check_output(cmd)
    decoded_output = codecs.decode(output, "utf-8")
    return decoded_output.strip()


def GetBazelOutputDir() -> str:
    """Return the Bazel output directory.

    This is described in https://bazel.build/remote/output-directories"""
    repo_cache_dir = Path(GetBazelRepositoryCacheDir())
    # The repository cache is inside the output directory, so going up
    # three levels returns the output directory.
    output_dir = repo_cache_dir.parent.parent.parent
    return str(output_dir)


def GetBazelWorkspaceCacheDir() -> str:
    """Determine the output directory cache for this workspace.

    Note: The Bazel docs(1) are very clear that the organization of the output
    directory may change at any time.

    (1) https://bazel.build/remote/output-directories
    """
    return os.path.join(GetBazelOutputDir(), GetBazelWorkspaceHash())


def GetMacSDKSymlinkDir() -> str:
    """Determine the MacSDK symlinks directory for this workspace."""
    return os.path.join(GetBazelWorkspaceCacheDir(), "external", "clang_mac", "symlinks", "xcode", "MacSDK")


if "__main__" == __name__:
    print(GetMacSDKSymlinkDir())
