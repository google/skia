"""This module defines the android_unit_test macro."""

load("//bazel:android_test.bzl", "android_test")

def android_unit_test(**kwargs):
    """Defines an Android unit test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android unit tests. See the android_test macro documentation for details.

    Args:
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    android_test(
        test_runner_if_required_condition_is_satisfied = "//tests:BazelTestRunner.cpp",
        test_runner_if_required_condition_is_not_satisfied = "//tests:BazelNoopRunner.cpp",
        **kwargs
    )
