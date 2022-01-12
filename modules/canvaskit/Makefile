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

release_viewer:
	# Does an incremental build where possible.
	./compile.sh viewer
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./build/

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

debug_cpu:
	# Does an incremental build where possible.
	./compile.sh debug cpu_only
	- rm -rf build/
	mkdir build
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./build/
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./build/

debug_viewer:
	# Does an incremental build where possible.
	./compile.sh debug viewer
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
	./compile.sh release no_skottie no_particles no_rt_shader no_sksl_trace no_alias_font no_effects_deserialization
	cp ../../out/canvaskit_wasm/canvaskit.js       ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./npm_build/bin

	mkdir -p ./npm_build/bin/profiling
	./compile.sh profiling
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./npm_build/bin/profiling
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./npm_build/bin/profiling

gm_tests_debug:
	./compile_gm.sh debug
	mkdir -p ./out
	cp ../../out/wasm_gm_tests_debug/wasm_gm_tests.js       ./out
	cp ../../out/wasm_gm_tests_debug/wasm_gm_tests.wasm     ./out

gm_tests:
	./compile_gm.sh
	mkdir -p ./out
	cp ../../out/wasm_gm_tests/wasm_gm_tests.js       ./out
	cp ../../out/wasm_gm_tests/wasm_gm_tests.wasm     ./out

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

bazel_gms_release:
	# We use spawn_strategy=local for "everyday" builds because emscripten assumes there
	# is a cache in the home directory that it needs to fill with compiled versions of libc etc.
	# https://emscripten.org/docs/tools_reference/emcc.html
	# By setting spawn_strategy=local, we can avoid recompiling all of this for every compilation
	# unit, by letting the cache be used (and not dropped from the sandbox), which gets expensive.
	# Local testing showed using the local strategy sped up a clean build from 9.5 minutes
	# to 1 minute. https://docs.bazel.build/versions/main/user-manual.html#strategy-options
	bazelisk build :gm_bindings_wasm --compilation_mode opt --spawn_strategy=local
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/gm_bindings_wasm/gm_bindings.js build/gm_bindings.js
	cp ../../bazel-bin/modules/canvaskit/gm_bindings_wasm/gm_bindings.wasm build/gm_bindings.wasm

bazel_gms_debug:
	# See above note about spawn_strategy
	bazelisk build :gm_bindings_wasm --compilation_mode dbg --spawn_strategy=local
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/gm_bindings_wasm/gm_bindings.js build/gm_bindings.js
	cp ../../bazel-bin/modules/canvaskit/gm_bindings_wasm/gm_bindings.wasm build/gm_bindings.wasm

bazel_canvaskit_debug:
	# See above note about spawn_strategy
	bazelisk build :canvaskit_wasm --compilation_mode dbg --spawn_strategy=local
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/canvaskit_wasm/canvaskit.js build/canvaskit.js
	cp ../../bazel-bin/modules/canvaskit/canvaskit_wasm/canvaskit.wasm build/canvaskit.wasm

bazel_canvaskit_release:
	# See above note about spawn_strategy
	bazelisk build :canvaskit_wasm --compilation_mode opt --spawn_strategy=local
	- rm -rf build/
	mkdir build
	cp ../../bazel-bin/modules/canvaskit/canvaskit_wasm/canvaskit.js build/canvaskit.js
	cp ../../bazel-bin/modules/canvaskit/canvaskit_wasm/canvaskit.wasm build/canvaskit.wasm
	ls -l build
