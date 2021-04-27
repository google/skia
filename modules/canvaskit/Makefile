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

strip_half:
	# Does an incremental build where possible.
	./compile.sh no_skottie no_particles no_font
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

strip_all:
	# Does an incremental build where possible.
	./compile.sh no_skp_serialization \
				no_effects_deserialization \
				no_skottie \
				no_managed_skottie \
				no_particles \
				no_pathops \
				no_rt_shader \
				no_matrix \
				no_canvas \
				no_font \
				no_embedded_font \
				no_woff2 \
				no_alias_font \
				no_paragraph \
				no_codecs \
				no_encode_png
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

bluescape_release:
	# Does an incremental build where possible.
	# Note that we shouldn't disable: no_fonts no_paragraph primitive_shaper no_embedded_font
	rm -rf ../../out/canvaskit_wasm
	./compile.sh \
				no_skp_serialization \
				no_effects_deserialization \
				no_skottie \
				no_managed_skottie \
				no_particles \
				no_pathops \
				no_rt_shader \
				no_matrix \
				no_canvas \
				no_embedded_font \
				no_woff2 \
				no_alias_font \
				no_codecs \
				no_encode_png
	rm -rf ./canvaskit/bin
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvaskit/bin

bluescape_debug:
	# Does an incremental build where possible.
	# Note that we shouldn't disable: no_fonts no_paragraph primitive_shaper
	rm -rf ../../out/canvaskit_wasm_debug
	./compile.sh debug \
				no_skp_serialization \
				no_effects_deserialization \
				no_skottie \
				no_managed_skottie \
				no_particles \
				no_pathops \
				no_rt_shader \
				no_matrix \
				no_canvas \
				no_embedded_font \
				no_woff2 \
				no_alias_font \
				no_codecs \
				no_encode_png
	rm -rf ./canvaskit-debug/
	cp -r ./canvaskit canvaskit-debug
	mkdir -p ./canvaskit-debug/bin
	sed -i -e 's/@bluescape\/canvaskit-wasm/@bluescape\/canvaskit\-debug\-wasm/g' ./package.json
	sed -i -e 's/\-bluescape\-/\-bluescape\-debug\-/g' canvaskit-debug/package.json
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit-debug/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit-debug/bin

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

experimental_simd:
	# Does an incremental build where possible.
	./compile.sh simd
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm ./canvaskit/bin
	cp ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm.map ./canvaskit/bin

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
	./compile.sh release no_skottie no_particles no_rt_shader no_alias_font no_effects_deserialization
	cp ../../out/canvaskit_wasm/canvaskit.js       ./canvaskit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./canvaskit/bin

	mkdir -p ./canvaskit/bin/core
	./compile.sh release no_skottie no_particles no_pathops no_rt_shader no_font no_skp_serialization no_effects_deserialization
	cp ../../out/canvaskit_wasm/canvaskit.js       ./canvaskit/bin/core
	cp ../../out/canvaskit_wasm/canvaskit.wasm     ./canvaskit/bin/core

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
