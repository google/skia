# This shows an example invocation presuming the user has built canvaskit locally.
frames_load_lego_with_local:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../modules/canvaskit/node_modules/canvaskit/bin/canvaskit.js \
	--canvaskit_wasm ../../modules/canvaskit/node_modules/canvaskit/bin/canvaskit.wasm --use_gpu \
	--input_lottie ../../modules/canvaskit/perf/assets/lego_loader.json \
	--bench_html skottie-frames.html

test_path_translating:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../modules/canvaskit/node_modules/canvaskit/bin/canvaskit.js \
	--canvaskit_wasm ../../modules/canvaskit/node_modules/canvaskit/bin/canvaskit.wasm --use_gpu \
	--bench_html path-translating.html