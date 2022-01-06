debug:
	bazelisk build //experimental/wasm-hello-world:hello_world_wasm --sandbox_base=/dev/shm
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/experimental/wasm-hello-world/hello_world_wasm/hello_world.js build/hello_world.js
	cp ../../bazel-bin/experimental/wasm-hello-world/hello_world_wasm/hello_world.wasm build/hello_world.wasm

serve:
	echo "Go check out http://localhost:8000/hello_world.html"
	python3 ../../tools/serve_wasm.py