# This shows an example invocation presuming the user has built canvaskit locally.
frames_load_lego_with_local:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_lottie ../../modules/canvaskit/perf/assets/lego_loader.json \
	--bench_html skottie-frames.html

test_path_transform_without_snap:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--assets path_translate_assets \
	--bench_html path-transform.html \
	--query_params translate opacity

test_path_transform_with_snap:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--assets path_translate_assets \
	--bench_html path-transform.html \
	--query_params translate opacity snap

skp_with_local:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_skp ${HOME}/skps/desk_nytimes.skp \
	--bench_html render-skp.html


skps_release:
	@for f in $(shell ls ${HOME}/skps); do node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_skp ${HOME}/skps/$${f} \
	--bench_html render-skp.html \
	--output release_out.json; done

skps_SIMD:
	@for f in $(shell ls ${HOME}/skps); do node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm_experimental_simd/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm --use_gpu \
	--input_skp ${HOME}/skps/$${f} \
	--bench_html render-skp.html \
	--output simd_out.json; done

skps_release_and_SIMD:
	@for f in $(shell ls ${HOME}/skps); do node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_skp ${HOME}/skps/$${f} \
	--bench_html render-skp.html \
	--chromium_executable_path "/applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary" \
	--overwrite_existing_output_property $${f} \
	--output release_out.json && \
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm_experimental_simd/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm_experimental_simd/canvaskit.wasm --use_gpu \
	--input_skp ${HOME}/skps/$${f} \
	--bench_html render-skp.html \
	--chromium_executable_path "/applications/Google Chrome Canary.app/Contents/MacOS/Google Chrome Canary" \
	--overwrite_existing_output_property $${f} \
	--output simd_out.json; done