This directory contains generated Go wrappers for Bazel cquery result
protocol buffers defined in https://github.com/bazelbuild/bazel/tree/master/src/main/protobuf.

An attempt was made to use [go_proto_library](https://github.com/bazelbuild/rules_go/blob/master/proto/core.rst#go-proto-library)
to generate this code at build time, sourcing the embedded_tools dependency, but that
was never successful. The cause appears to be that the protobufs in Bazel's source
defined messages of the same name (specifically "Target") which creates a build
conflict. The command below generates the two Go classes with different package names
to avoid this conflict - which is what the Bazel generated Java wrapper does.

They were generated as so:

```bash
BAZEL_DIR=/path/to/bazel/source
DST_DIR=${PWD}/bazel/exporter/build_proto
GO_PACKAGE=go.skia.org/skia/bazel/exporter/build_proto
GO_GEN_CODE_ROOT=${DST_DIR}/go.skia.org/skia/bazel/exporter/build_proto

protoc \
  --proto_path=${BAZEL_DIR} \
  --go_out=${DST_DIR} \
  --go_opt=Msrc/main/protobuf/build.proto=${GO_PACKAGE}/build \
  --go_opt=Msrc/main/protobuf/analysis_v2.proto=${GO_PACKAGE}/analysis_v2 \
  ${SRC_DIR}/analysis_v2.proto ${SRC_DIR}/build.proto
```

The call above writes the generated code to `${DST_DIR}/go.skia.org/skia/bazel/exporter/build_proto`
which is then moved into `${DST_DIR}/build` and `${DST_DIR}/build`.