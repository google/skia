"""This module defines the android_unit_test macro."""

load("//tools/testrunners/common/android:android_test.bzl", "android_test")

def android_unit_test(deps = [], **kwargs):
    """Defines an Android unit test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android unit tests. See the android_test macro documentation for details.

    Args:
        deps: Dependencies for this test to run.
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    android_test(deps = deps + [
        "//tools/testrunners/unit:testrunner",
    ], **kwargs)
