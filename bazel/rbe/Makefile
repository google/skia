LINUX_VERSION=v2

build_linux_container:
	docker build -t gcr.io/skia-public/rbe_linux:${LINUX_VERSION} ./gce_linux_container/

push_linux_container: build_linux_container
	docker push gcr.io/skia-public/rbe_linux:${LINUX_VERSION}

generate_linux_config:
	# If you do not have the rbe_configs_gen executable, see
	# https://github.com/bazelbuild/bazel-toolchains/releases/tag/v5.1.2
	rbe_configs_gen \
		--bazel_version=5.0.0 \
		--toolchain_container=gcr.io/skia-public/rbe_linux@sha256:654139e5cecb163f80a7d18e2b2da6c758ebe6325d2d9c41d9facf58e1b3f799 \
		--output_src_root=../.. \
		--output_config_path=bazel/rbe/gce_linux \
		--exec_os=linux \
		--target_os=linux