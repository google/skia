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
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./canvaskit/bin

debug_cpu:
	# Does an incremental build where possible.
	./compile.sh debug cpu_only
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./canvaskit/bin

debug_viewer:
	# Does an incremental build where possible.
	./compile.sh debug viewer
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm.map ./canvaskit/bin

profile:
	./compile.sh profiling
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.js       ./canvaskit/bin
	cp ../../out/canvaskit_wasm_profile/canvaskit.wasm     ./canvaskit/bin

npm:
	rm -rf ./canvaskit/bin
	mkdir -p ./canvaskit/bin
	# These features are turned off to keep code size smaller for the
	# general use case.
	./compile.sh release no_skottie no_particles no_pathops no_rt_shader no_skp
	cp ../../out/canvaskit_wasm/canvaskit.js       ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./canvaskit/bin

	mkdir -p ./canvaskit/bin/core
	./compile.sh release no_skottie no_particles no_pathops no_rt_shader no_font no_skp
	cp ../../out/canvaskit_wasm/canvaskit.js       ./canvaskit/bin/core
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./canvaskit/bin/core

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

.PHONY: perf
perf:
	npx karma start ./karma.bench.conf.js --single-run

node-example:
	node ./canvaskit/node.example.js --expose-wasm

docker-compile:
	mkdir -p ${SKIA_ROOT}/out/canvaskit_wasm_docker
	docker run --rm --volume ${SKIA_ROOT}:/SRC \
               --volume ${SKIA_ROOT}/out/canvaskit_wasm_docker:/OUT \
               gcr.io/skia-public/canvaskit-emsdk:1.39.6_v1 \
               /SRC/infra/canvaskit/build_canvaskit.sh
