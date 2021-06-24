This folder runs perf tests against CanvasKit.

## Initial setup

Run `npm ci` to install the dependencies need to run the tests. In //modules/canvaskit, run
`make release`, `make profile`, or `make debug` to build the canvaskit that will be used. It is
recommended you download the "lottie-samples" asset using the sk tool: 
`sk asset download lottie-samples ~/Downloads/lottie-samples`. They can go anywhere.

## Testing a single skottie file

The Makefile has some examples for invoking the test harness. Your command should look something
like:

    node skottie-wasm-perf.js --canvaskit_js ../../out/canvaskit_wasm/canvaskit.js
	    --canvaskit_wasm ../../out/canvaskit_wasm/canvaskit.wasm
        --use_gpu
	    --input ~/Downloads/lottie-samples/404.json
	    --output output.json
        --timeout 200

Be sure to update the `--canvaskit_js` and `--canvaskit_wasm` flags to reflect the build of 
canvaskit you want to actually use. For example, canvaskit_wasm for `make relase`, 
canvaskit_wasm_debug for `make debug`, etc.

Setting the timeout flag to a very large number can be handy if you want to dive in and set
breakpoints.

