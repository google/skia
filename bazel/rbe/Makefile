LINUX_VERSION=v3

build_linux_container:
	docker build -t gcr.io/skia-public/rbe_linux:${LINUX_VERSION} ./gce_linux_container/

push_linux_container: build_linux_container
	docker push gcr.io/skia-public/rbe_linux:${LINUX_VERSION}

shell_into_linux_container:
	docker run -it gcr.io/skia-public/rbe_linux:${LINUX_VERSION} /bin/bash

generate_linux_config:
	# If you do not have the rbe_configs_gen executable, see
	# https://github.com/bazelbuild/bazel-toolchains/releases/tag/v5.1.2
	rbe_configs_gen \
		--bazel_version=5.3.2 \
		--toolchain_container=gcr.io/skia-public/rbe_linux@sha256:82e8a4c7d06c8f47bbc08ee899c4c03069af0f7f4d8c0d958a50a23d814405e6 \
		--output_src_root=../.. \
		--output_config_path=bazel/rbe/gce_linux \
		--exec_os=linux \
		--target_os=linux
