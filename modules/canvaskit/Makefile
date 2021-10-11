clean:
	rm -rf ../../out/canvaskit_wasm
	rm -rf ./npm_build/bin
	$(MAKE) release

release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./npm_build/bin

release_cpu:
	# Does an incremental build where possible.
	./compile.sh cpu_only
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./npm_build/bin

release_viewer:
	# Does an incremental build where possible.
	./compile.sh viewer
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./npm_build/bin

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./npm_build/bin
	#cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./npm_build/bin

debug_cpu:
	# Does an incremental build where possible.
	./compile.sh debug cpu_only
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./npm_build/bin
	#cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./npm_build/bin

experimental_simd:
	# Does an incremental build where possible.
	./compile.sh simd
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm ./npm_build/bin

debug_viewer:
	# Does an incremental build where possible.
	./compile.sh debug viewer
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./npm_build/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./npm_build/bin

profile:
	./compile.sh profiling
	mkdir -p ./npm_build/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./npm_build/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./npm_build/bin

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
	./compile.sh release no_skottie no_particles no_rt_shader no_alias_font no_effects_deserialization
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
	rm -rf node_modules/canvaskit
	mkdir -p node_modules
	ln -s ../npm_build node_modules/canvaskit
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
