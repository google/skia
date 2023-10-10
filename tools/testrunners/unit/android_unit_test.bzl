"""This module defines the android_unit_test macro."""

load("//tools/testrunners/common/android:android_test.bzl", "android_test")

def android_unit_test(**kwargs):
    """Defines an Android unit test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android unit tests. See the android_test macro documentation for details.

    Args:
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    android_test(
        test_runner_if_required_condition_is_satisfied = "//tools/testrunners/unit:testrunner",
        test_runner_if_required_condition_is_not_satisfied = "//tools/testrunners/common:noop_testrunner",
        **kwargs
    )
