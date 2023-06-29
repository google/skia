"""This module defines the android_unit_test macro."""

load("//bazel:android_test.bzl", "android_test")

def android_unit_test(**kwargs):
    """Defines an Android unit test.

    This macro is just a wrapper around the android_test macro. See that macro's documentation for
    details.

    Args:
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    android_test(**kwargs)
