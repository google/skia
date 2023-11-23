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

# Run this target to test all known working Bazel builds
known_good_builds:
	bazelisk build //:skia_public
	bazelisk build //:skia_internal
	bazelisk build //experimental/bazel_test/...
	bazelisk run //experimental/bazel_test:bazel_test_exe
	bazelisk build //example:hello_world_gl
	bazelisk build //example:hello_world_vulkan
	bazelisk build //example:hello_world_dawn
	bazelisk build //example:vulkan_basic
	bazelisk build //src/sksl/lex:sksllex
	bazelisk build //tools/skdiff
	bazelisk build //tools/skslc
	bazelisk build //modules/skottie:skottie_tool_cpu
	bazelisk build //modules/skottie:skottie_tool_gpu
	bazelisk build //modules/canvaskit:canvaskit --config=ck_full_webgl2_release
	bazelisk build //modules/pathkit:pathkit --config=ck_full_webgl2_release

rbe_known_good_builds:
	bazelisk build //:skia_public --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //:skia_internal --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //experimental/bazel_test:bazel_test_exe --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //example:hello_world_gl --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //example:hello_world_vulkan --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //example:hello_world_dawn --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //example:vulkan_basic --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //src/sksl/lex:sksllex --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //tools/skdiff --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //tools/skslc --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //modules/skottie:skottie_tool_cpu --config=for_linux_x64_with_rbe --remote_download_minimal
	bazelisk build //modules/skottie:skottie_tool_gpu --config=for_linux_x64_with_rbe --remote_download_minimal
	## TODO(kjlubick) CanvasKit in release mode (i.e. with Closure) requires
	## https://github.com/emscripten-core/emscripten/pull/16640 to land
	bazelisk build //modules/canvaskit:canvaskit --config=linux_rbe \
		--config=ck_full_webgl2_debug --remote_download_minimal
	bazelisk build //modules/pathkit:pathkit --config=linux_rbe \
		--config=ck_full_webgl2_debug --remote_download_minimal

rbe_known_good_tests:
	bazel test //tests/... --config=cpu_only_debug --config=linux_rbe \
		--remote_download_minimal --test_output=errors

iwyu_rbe:
	bazelisk build //:skia_public --config=for_linux_x64_with_rbe \
		--config=enforce_iwyu --remote_download_minimal
	bazelisk build //modules/skottie:skottie_tool_gpu --config=for_linux_x64_with_rbe \
		--config=enforce_iwyu --remote_download_minimal
	bazelisk build //tests/... --config=for_linux_x64_with_rbe \
		--config=enforce_iwyu --remote_download_minimal

iwyu:
	bazelisk build //:skia_public --config=enforce_iwyu
	bazelisk build //modules/skottie:skottie_tool_gpu --config=enforce_iwyu
	bazelisk build //tests/... --config=enforce_iwyu
