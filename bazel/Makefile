EXPORTER_TOOL=bazel-bin/bazel/exporter_tool/exporter_tool_/exporter_tool
EXPORTER_RULES=-rule='//src/...' -rule='//modules/...' -rule='//include/...' -rule='//resources/...' -rule='//experimental/rust_png/...'

generate_go:
	cd .. && bazelisk run //infra:gazelle -- update infra/bots/task_drivers modules/canvaskit/go \
       bazel/device_specific_configs bazel/exporter bazel/deps_parser tools/gpu/gl/interface \
			 tools/testrunners/common/make_tarball tools/testrunners/common/android/adb_test_runner

.PHONY: generate_cmake
generate_cmake:
	cd .. && bazelisk build //bazel/exporter_tool && ${EXPORTER_TOOL} \
		-output_format=cmake -proj_name=Skia -rule='//:core'

.PHONY: generate_gni
generate_gni:
	# It is tempting to try to put this into a blaze run command. However, that
	# is currently infeasible due to the fact that the exporter tool *calls*
	# Bazel, resulting in a deadlock because only one Bazel command can run
	# at a time.
	cd .. && bazelisk build //bazel/exporter_tool && \
        ${EXPORTER_TOOL} -output_format=gni -proj_name=Skia ${EXPORTER_RULES} \
		&& bazelisk run //bazel/external/gn -- format gn/*.gni modules/*/*.gni

HOST_GO_ARCH := $(shell bazelisk run //:go -- version | awk '{print $$NF}' | sed 's/\//_/g')

.PHONY: generate_gni_rbe
generate_gni_rbe:
	cd .. && bazelisk build //bazel/exporter_tool --config=linux_rbe --jobs=100 \
 		--platforms=@rules_go//go/toolchain:$(HOST_GO_ARCH) \
 		--remote_download_toplevel && \
        ${EXPORTER_TOOL} -output_format=gni -proj_name=Skia ${EXPORTER_RULES} \
		&& bazelisk run //bazel/external/gn -- format gn/*.gni modules/*/*.gni

.PHONY: mocks
mocks:
	go generate ./...

generate_third_party_patches:
	cd .. && python3 tools/generate_patches.py \
	third_party/freetype2/include/freetype-android/freetype/config/ftmodule.h builds/android-ftmodule.h \
	third_party/freetype2/include/freetype-android/freetype/config/ftoption.h builds/android-ftoption.h \
	third_party/freetype2/include/freetype-no-type1/freetype/config/ftmodule.h builds/no-type1-ftmodule.h \
	third_party/freetype2/include/freetype-no-type1/freetype/config/ftoption.h builds/no-type1-ftoption.h \
	> bazel/external/freetype/config_files.patch

	cd .. && python3 tools/generate_patches.py \
	third_party/expat/include/expat_config/expat_config.h expat_config.h \
	> bazel/external/expat/config_files.patch

	cd .. && python3 tools/generate_patches.py \
	third_party/harfbuzz/config-override.h config-override.h \
	> bazel/external/harfbuzz/config_files.patch

	cd .. && python3 tools/generate_patches.py \
	third_party/icu/SkLoadICU.cpp SkLoadICU.cpp \
	third_party/icu/SkLoadICU.h SkLoadICU.h \
	third_party/icu/make_data_cpp.py make_data_cpp.py \
	> bazel/external/icu/icu_utils.patch