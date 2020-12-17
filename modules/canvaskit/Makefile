clean:
	rm -rf ../../out/canvaskit_wasm
	rm -rf ./canvaskit/bin
	$(MAKE) release

release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

release_cpu:
	# Does an incremental build where possible.
	./compile.sh cpu_only
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

release_viewer:
	# Does an incremental build where possible.
	./compile.sh viewer
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin
	#cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./canvaskit/bin

debug_cpu:
	# Does an incremental build where possible.
	./compile.sh debug cpu_only
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin
	#cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./canvaskit/bin

experimental_simd:
	# Does an incremental build where possible.
	./compile.sh simd
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm ./canvaskit/bin

debug_viewer:
	# Does an incremental build where possible.
	./compile.sh debug viewer
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin

profile:
	./compile.sh profiling
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./canvaskit/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./canvaskit/bin

npm:
	rm -rf ./canvaskit/bin
	mkdir -p ./canvaskit/bin
	cp ./CHANGELOG.md ./canvaskit/
	# These features are turned off to keep code size smaller for the
	# general use case.
	./compile.sh release no_skottie no_particles no_rt_shader no_alias_font no_effects_deserialization
	cp ../../out/canvaskit_wasm/canvaskit.js       ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./canvaskit/bin

	mkdir -p ./canvaskit/bin/profiling
	./compile.sh profiling no_skottie no_particles no_rt_shader no_alias_font no_effects_deserialization
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./canvaskit/bin/profiling
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./canvaskit/bin/profiling

gm_tests:
	./compile_gm.sh
	mkdir -p ./out
	cp ../../out/wasm_gm_tests/wasm_gm_tests.js       ./out
	cp ../../out/wasm_gm_tests/wasm_gm_tests.wasm     ./out

local-example:
	rm -rf node_modules/canvaskit
	mkdir -p node_modules
	ln -s ../canvaskit node_modules/canvaskit
	echo "Go check out http://localhost:8000/canvaskit/example.html"
	python serve.py

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make debug or release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

test-continuous-headless:
	npx karma start ./karma.conf.js --no-single-run --watch-poll --headless

.PHONY: perf
perf:
	npx karma start ./karma.bench.conf.js --single-run

node-example:
	node ./canvaskit/node.example.js --expose-wasm

docker-compile:
	mkdir -p ${SKIA_ROOT}/out/canvaskit_wasm_docker
	docker run --rm --volume ${SKIA_ROOT}:/SRC \
               --volume ${SKIA_ROOT}/out/canvaskit_wasm_docker:/OUT \
               gcr.io/skia-public/canvaskit-emsdk:2.0.0_v1 \
               /SRC/infra/canvaskit/build_canvaskit.sh

typecheck:
	echo "Make sure you've run cd canvaskit && npm ci recently"
	cd canvaskit && npm run dtslint
