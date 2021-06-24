# This shows an example invocation presuming the user has built canvaskit locally using
# make release
release_perf:
	node skottie-wasm-perf.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm --use_gpu \
	--input ~/Downloads/lottie-samples/lottiefiles.com\ -\ Octopus.json \
	--output octopus_perf.json \
    --timeout 120
# This shows an example invocation presuming the user has built canvaskit locally using
# make release
profile_perf:
	node skottie-wasm-perf.js --canvaskit_js ../../out/canvaskit_wasm_profile/canvaskit.js \
	--canvaskit_wasm ../../out/canvaskit_wasm_profile/canvaskit.wasm --use_gpu \
	--input ~/Downloads/lottie-samples/lottiefiles.com\ -\ Octopus.json \
	--output octopus_perf.json \
	--timeout 120