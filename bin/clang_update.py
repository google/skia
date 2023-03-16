#!/usr/bin/env python3
# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""
Bits and pieces of Chromium's [1] tools/clang/scripts/update.py needed
for fetching Rust toolchain, see fetch-rust-toolchain.

[1] https://source.chromium.org/chromium/chromium/src/+/main:tools/clang/scripts/update.py

"""

import argparse
import os
import platform
import shutil
import stat
import tarfile
import tempfile
import time
import urllib.request
import urllib.error
import zipfile
import zlib
import sys


CDS_URL = os.environ.get(
    "CDS_CLANG_BUCKET_OVERRIDE",
    "https://commondatastorage.googleapis.com/chromium-browser-clang",
)


def EnsureDirExists(path):
    if not os.path.exists(path):
        os.makedirs(path)


def DownloadAndUnpack(url, output_dir, path_prefixes=None):
    """Download an archive from url and extract into output_dir. If path_prefixes
    is not None, only extract files whose paths within the archive start with
    any prefix in path_prefixes."""
    with tempfile.TemporaryFile() as f:
        DownloadUrl(url, f)
        f.seek(0)
        EnsureDirExists(output_dir)
        if url.endswith(".zip"):
            assert path_prefixes is None
            zipfile.ZipFile(f).extractall(path=output_dir)
        else:
            t = tarfile.open(mode="r:*", fileobj=f)
            members = None
            if path_prefixes is not None:
                members = [
                    m
                    for m in t.getmembers()
                    if any(m.name.startswith(p) for p in path_prefixes)
                ]
            t.extractall(path=output_dir, members=members)


def GetPlatformUrlPrefix(host_os):
    _HOST_OS_URL_MAP = {
        "linux": "Linux_x64",
        "mac": "Mac",
        "mac-arm64": "Mac_arm64",
        "win": "Win",
    }
    return CDS_URL + "/" + _HOST_OS_URL_MAP[host_os] + "/"


def DownloadUrl(url, output_file):
    """Download url into output_file."""
    CHUNK_SIZE = 4096
    TOTAL_DOTS = 10
    num_retries = 3
    retry_wait_s = 5  # Doubled at each retry.

    while True:
        try:
            sys.stdout.write("Downloading %s " % url)
            sys.stdout.flush()
            request = urllib.request.Request(url)
            request.add_header("Accept-Encoding", "gzip")
            response = urllib.request.urlopen(request)
            total_size = None
            if "Content-Length" in response.headers:
                total_size = int(response.headers["Content-Length"].strip())

            is_gzipped = response.headers.get("Content-Encoding", "").strip() == "gzip"
            if is_gzipped:
                gzip_decode = zlib.decompressobj(zlib.MAX_WBITS + 16)

            bytes_done = 0
            dots_printed = 0
            while True:
                chunk = response.read(CHUNK_SIZE)
                if not chunk:
                    break
                bytes_done += len(chunk)

                if is_gzipped:
                    chunk = gzip_decode.decompress(chunk)
                output_file.write(chunk)

                if total_size is not None:
                    num_dots = TOTAL_DOTS * bytes_done // total_size
                    sys.stdout.write("." * (num_dots - dots_printed))
                    sys.stdout.flush()
                    dots_printed = num_dots
            if total_size is not None and bytes_done != total_size:
                raise urllib.error.URLError(
                    "only got %d of %d bytes" % (bytes_done, total_size)
                )
            if is_gzipped:
                output_file.write(gzip_decode.flush())
            print(" Done.")
            return
        except urllib.error.URLError as e:
            sys.stdout.write("\n")
            print(e)
            if (
                num_retries == 0
                or isinstance(e, urllib.error.HTTPError)
                and e.code == 404
            ):
                raise e
            num_retries -= 1
            output_file.seek(0)
            output_file.truncate()
            print("Retrying in %d s ..." % retry_wait_s)
            sys.stdout.flush()
            time.sleep(retry_wait_s)
            retry_wait_s *= 2


def GetDefaultHostOs():
    _PLATFORM_HOST_OS_MAP = {
        "darwin": "mac",
        "cygwin": "win",
        "linux2": "linux",
        "win32": "win",
    }
    default_host_os = _PLATFORM_HOST_OS_MAP.get(sys.platform, sys.platform)
    if default_host_os == "mac" and platform.machine() == "arm64":
        default_host_os = "mac-arm64"
    return default_host_os
