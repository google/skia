# This shows an example invocation presuming the user has built canvaskit locally using
# make release
local_perf_release:
	node skottie-wasm-perf.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input ~/Downloads/lottie-samples/lottiefiles.com\ -\ Octopus.json \
	--output octopus_perf.json

# This shows an example invocation presuming the user has built canvaskit locally using
# make release
local_perf_profile:
	node skottie-wasm-perf.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm_profile/canvaskit.wasm --use_gpu \
	--input ~/Downloads/lottie-samples/lottiefiles.com\ -\ Octopus.json \
	--output octopus_perf.json