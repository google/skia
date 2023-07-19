"""This module provides the gcs_mirror_url macro."""

# Set to True to force the macro to only return the mirror URL.
_TEST_GCS_MIRROR = False

# Must be kept in sync with the suffixes supported by gcs_mirror (e.g.
# https://skia.googlesource.com/skia/+/8ad66c2340713234df6b249e793415233337a103/bazel/gcs_mirror/gcs_mirror.go#140).
_SUPPORTED_SUFFIXES = [".tar.gz", ".tgz", ".tar.xz", ".deb", ".zip"]

_GCS_MIRROR_PREFIX = "https://storage.googleapis.com/skia-world-readable/bazel"

def gcs_mirror_url(url, sha256, ext = None):
    """Takes the URL of an external resource and computes its GCS mirror URL.

    We store backup copies of external resources in the skia-world-readable GCS bucket. This macro
    returns a list with two elements: the original URL, and the mirrored URL.

    To mirror a new URL, please use the `gcs_mirror` utility found at
    https://skia.googlesource.com/skia/+/8ad66c2340713234df6b249e793415233337a103/bazel/gcs_mirror/gcs_mirror.go.

    Args:
        url: URL of the mirrored resource.
        sha256: SHA256 hash of the mirrored resource.
        ext: string matching the extension, if not provided, it will be gleaned from the URL.
             The auto-detected suffix must match a list. An arbitrarily provided one does not.
    Returns:
        A list of the form [original URL, mirror URL].
    """
    extension = ""
    if ext == None:
        for suffix in _SUPPORTED_SUFFIXES:
            if url.endswith(suffix):
                extension = suffix
                break
        if extension == "":
            fail("URL %s has an unsupported suffix." % url)

    mirror_url = "%s/%s%s" % (_GCS_MIRROR_PREFIX, sha256, extension)
    return [mirror_url] if _TEST_GCS_MIRROR else [mirror_url, url]
