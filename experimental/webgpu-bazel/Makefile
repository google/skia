release:
	bazel build //experimental/webgpu-bazel/src:hello-world-wasm --compilation_mode opt
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/experimental/webgpu-bazel/src/hello-world-wasm/hello-world.js build/hello-world.js
	cp ../../bazel-bin/experimental/webgpu-bazel/src/hello-world-wasm/hello-world.wasm build/hello-world.wasm

debug:
	bazel build //experimental/webgpu-bazel/src:hello-world-wasm --compilation_mode dbg
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/experimental/webgpu-bazel/src/hello-world-wasm/hello-world.js build/hello-world.js
	cp ../../bazel-bin/experimental/webgpu-bazel/src/hello-world-wasm/hello-world.wasm build/hello-world.wasm

serve:
	python3 ../../tools/serve_wasm.py