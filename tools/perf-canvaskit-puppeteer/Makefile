# All of the targets here assume a user has run `make release` in //modules/canvaskit

perf_js:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--assets canvas_perf_assets \
	--bench_html canvas_perf.html \
	--timeout 600 # seconds

frames:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_lottie ~/Downloads/lottie-samples/stacking.json \
	--bench_html skottie-frames.html

frames_cpu:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm \
	--input_lottie ~/Downloads/lottie-samples/404.json  \
	--bench_html skottie-frames.html

skp:
	node perf-canvaskit-with-puppeteer.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input_skp ~/Downloads/skps/desk_nytimes.skp \
	--bench_html render-skp.html

skps_release_and_simd:
	./perf_all_skps.sh --release --simd --summary