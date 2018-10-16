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

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.js   ./canvaskit/bin
	cp ../../out/canvaskit_wasm_debug/canvaskit.wasm ./canvaskit/bin

local-example:
	rm -rf node_modules/canvaskit
	mkdir -p node_modules
	ln -s -T ../canvaskit node_modules/canvaskit
	echo "Go check out http://localhost:8000/canvaskit/example.html"
	python serve.py

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make debug or release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll