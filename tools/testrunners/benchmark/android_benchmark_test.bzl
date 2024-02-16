"""This module defines the android_benchmark_test macro."""

load("//tools/testrunners/common/android:android_test.bzl", "android_test")

def android_benchmark_test(args = [], **kwargs):
    """Defines an Android benchmark test.

    This macro is just a wrapper around the android_test macro with the necessary defaults for
    Android benchmark tests. See the android_test macro documentation for details.

    Args:
        args: Any command-line arguments to pass to the test.
        **kwargs: Any arguments to pass to the underlying android_test macro instance.
    """
    android_test(
        extra_args = args + [
            "--outputDir",
            # This environment variable is set by the adb_test_runner.go program.
            "$ADB_TEST_OUTPUT_DIR",
        ],
        benchmark = True,
        save_output_files = True,  # Save any produced PNG and JSON files as undeclared outputs.
        **kwargs
    )
