EXPORTER_TOOL=bazel-bin/bazel/exporter_tool/exporter_tool_/exporter_tool
EXPORTER_RULES=-rule='//src/...' -rule='//modules/...' -rule='//include/...' -rule='//resources/...'

generate:
	echo "You do not need to run this any more for C++ code, only if you modify go code"

generate_go:
	cd .. && bazelisk run //infra:gazelle -- update infra/bots/task_drivers modules/canvaskit/go \
       bazel/device_specific_configs bazel/exporter bazel/deps_parser tools/gpu/gl/interface \
			 tools/testrunners/common/make_tarball tools/testrunners/common/android/adb_test_runner \
			 experimental/bazel_test/go

.PHONY: generate_cmake
generate_cmake:
	cd .. && bazelisk build //bazel/exporter_tool && ${EXPORTER_TOOL} \
		-output_format=cmake -proj_name=Skia -rule='//:skia_public'

.PHONY: generate_gni
generate_gni:
	# It is tempting to try to put this into a blaze run command. However, that
	# is currently infeasible due to the fact that the exporter tool *calls*
	# Bazel, resulting in a deadlock because only one Bazel command can run
	# at a time.
	cd .. && bazelisk build //bazel/exporter_tool --config=no_sandbox && \
        ${EXPORTER_TOOL} -output_format=gni -proj_name=Skia ${EXPORTER_RULES} \
		&& bazelisk run //bazel/external/gn -- format gn/*.gni modules/*/*.gni

.PHONY: generate_gni_rbe
generate_gni_rbe:
	cd .. && bazelisk build //bazel/exporter_tool --config=linux_rbe --jobs=100 \
 		--platforms=@io_bazel_rules_go//go/toolchain:$(shell bazelisk run //bazel/utils:go_platform) \
 		--remote_download_toplevel && \
        ${EXPORTER_TOOL} -output_format=gni -proj_name=Skia ${EXPORTER_RULES} \
		&& bazelisk run //bazel/external/gn -- format gn/*.gni modules/*/*.gni

.PHONY: mocks
mocks:
	go generate ./...

# This target should be run after the go.mod file is updated (e.g. version rolls or new updates)
gazelle_update_repo:
	cd .. && bazelisk run //infra:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories -prune
