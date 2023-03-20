This contains files what we can use to test our Bazel toolchains or prototype new BUILD rules.

One can test the layering_check implementation with the following 4 commands, expecting each to fail
    bazel build //experimental/bazel_test/client:client_lib \
        --copt=-DHEADER_INCLUDES_TRANSITIVE_HEADER=1
    bazel build //experimental/bazel_test/client:client_lib \
        --copt=-DHEADER_INCLUDES_PRIVATE_HEADER=1
    bazel build //experimental/bazel_test/client:client_lib \
        --copt=-DSOURCE_INCLUDES_TRANSITIVE_HEADER=1
    bazel build //experimental/bazel_test/client:client_lib \
        --copt=-DSOURCE_INCLUDES_PRIVATE_HEADER=1