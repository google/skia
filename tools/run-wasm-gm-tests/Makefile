run_local_debug:
	rm -rf /tmp/wasm-gmtests/
	mkdir /tmp/wasm-gmtests/
	touch /tmp/wasm-gmtests/empty.txt
	node run-wasm-gm-tests --js_file ../../out/wasm_gm_tests_debug/wasm_gm_tests.js \
	--wasm_file ../../out/wasm_gm_tests_debug/wasm_gm_tests.wasm \
	--resources ../../resources \
	--known_hashes /tmp/wasm-gmtests/empty.txt \
	--output /tmp/wasm-gmtests/ \
	--use_gpu --timeout 600 \
	--manual_mode

run_local:
	rm -rf /tmp/wasm-gmtests/
	mkdir /tmp/wasm-gmtests/
	touch /tmp/wasm-gmtests/empty.txt
	node run-wasm-gm-tests --js_file ../../out/wasm_gm_tests/wasm_gm_tests.js \
	--wasm_file ../../out/wasm_gm_tests/wasm_gm_tests.wasm \
	--resources ../../resources \
	--known_hashes /tmp/wasm-gmtests/empty.txt \
	--output /tmp/wasm-gmtests/ \
	--use_gpu --timeout 220 \
	--manual_mode
