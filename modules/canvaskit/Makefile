clean:
	- rm -rf ../../out/canvaskit_wasm
	- rm -rf ./npm_build/bin
	- rm -rf ./build/
	$(MAKE) release

release:
	# Does an incremental build where possible.
	./compile.sh
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/

release_cpu:
	# Does an incremental build where possible.
	./compile.sh cpu_only
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/

release_webgpu:
	# Does an incremental build where possible.
	./compile.sh use_webgpu
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/

release_viewer:
	# Does an incremental build where possible.
	./compile.sh viewer
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/

debug:
	# Does an incremental build where possible.
	./compile.sh debug_build
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

debug_cpu:
	# Does an incremental build where possible.
	./compile.sh debug_build cpu_only
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

debug_webgpu:
	# Does an incremental build where possible.
	./compile.sh debug_build use_webgpu
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

debug_viewer:
	# Does an incremental build where possible.
	./compile.sh debug_build viewer
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

profile:
	./compile.sh profiling
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_profile/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm ./build/

npm:
	rm -rf ./npm_build/bin
	mkdir -p ./npm_build/bin
	cp ./CHANGELOG.md ./npm_build/

	mkdir -p ./npm_build/bin/full
	./compile.sh release
	cp ../../out/canvaskit_wasm/canvaskit.js       ./npm_build/bin/full
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./npm_build/bin/full

	# These features are turned off to keep code size smaller for the
	# general use case.
	./compile.sh release no_skottie no_sksl_trace no_alias_font \
		no_effects_deserialization no_encode_jpeg no_encode_webp legacy_draw_vertices \
		no_embedded_font
	cp ../../out/canvaskit_wasm/canvaskit.js       ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./npm_build/bin

	mkdir -p ./npm_build/bin/profiling
	./compile.sh profiling
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./npm_build/bin/profiling
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./npm_build/bin/profiling

gm_tests_debug:
	./compile_gm.sh debug
	- rm -rf build/
	mkdir build
	cp ../../out/wasm_gm_tests_debug/wasm_gm_tests.js       ./build
	cp ../../out/wasm_gm_tests_debug/wasm_gm_tests.wasm     ./build

gm_tests:
	./compile_gm.sh
	- rm -rf build/
	mkdir build
	cp ../../out/wasm_gm_tests/wasm_gm_tests.js       ./build
	cp ../../out/wasm_gm_tests/wasm_gm_tests.wasm     ./build

single-gm:
	echo "Go check out http://localhost:8000/wasm_tools/gms.html"
	python3 ../../tools/serve_wasm.py

local-example:
	echo "Go check out http://localhost:8000/npm_build/example.html"
	python3 ../../tools/serve_wasm.py

test-continuous:
	echo "Assuming npm ci has been run by user"
	echo "Also assuming make debug or release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

test-continuous-headless:
	npx karma start ./karma.conf.js --no-single-run --watch-poll --headless

node-example:
	node ./npm_build/node.example.js --expose-wasm

docker-compile:
	mkdir -p ${SKIA_ROOT}/out/canvaskit_wasm_docker
	docker run --rm --volume ${SKIA_ROOT}:/SRC \
               --volume ${SKIA_ROOT}/out/canvaskit_wasm_docker:/OUT \
               gcr.io/skia-public/canvaskit-emsdk:2.0.0_v1 \
               /SRC/infra/canvaskit/build_canvaskit.sh

typecheck:
	echo "Make sure you've run cd npm_build && npm ci recently"
	cd npm_build && npm run dtslint

bazel_canvaskit_debug:
	bazelisk build :canvaskit --config=ck_full_webgl2_debug
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.js build/canvaskit.js
	cp ../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.wasm build/canvaskit.wasm

bazel_canvaskit_release:
	bazelisk build :canvaskit --config=ck_full_webgl2_release
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.js build/canvaskit.js
	cp ../../bazel-bin/modules/canvaskit/canvaskit/canvaskit.wasm build/canvaskit.wasm
	ls -l build

bazel_test_canvaskit:
	bazelisk test :canvaskit_js_tests --config=ck_full_webgl2_release_chrome --test_output=streamed
	echo "test output in //bazel-testlogs/modules/canvaskit/canvaskit_js_tests/test.outputs/"
	cd ../../bazel-testlogs/modules/canvaskit/canvaskit_js_tests/test.outputs/ && unzip -qo outputs.zip && ls

with_debugger:
	# Does an incremental build where possible.
	./compile.sh debug_build enable_debugger
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/
	- mkdir ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./build/canvaskit.js         ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./build/canvaskit.wasm       ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./npm_build/types/index.d.ts ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/canvaskit.d.ts

with_debugger_release:
	./compile.sh enable_debugger
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/
	- mkdir ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./build/canvaskit.js         ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./build/canvaskit.wasm       ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/
	cp ./npm_build/types/index.d.ts ${SKIA_INFRA_ROOT}/debugger-app/wasm_libs/local_build/canvaskit.d.ts

for_shaders:
	./compile.sh
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/
	- mkdir ${SKIA_INFRA_ROOT}/shaders/wasm_libs/local_build/
	cp ./build/canvaskit.js         ${SKIA_INFRA_ROOT}/shaders/wasm_libs/local_build/
	cp ./build/canvaskit.wasm       ${SKIA_INFRA_ROOT}/shaders/wasm_libs/local_build/
	cp ./npm_build/types/index.d.ts ${SKIA_INFRA_ROOT}/shaders/wasm_libs/local_build/canvaskit.d.ts
