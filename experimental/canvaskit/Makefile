clean:
	rm -rf ../../out/canvaskit_wasm
	rm -rf ./canvas-kit/bin
	$(MAKE) release

release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./canvas-kit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvas-kit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvas-kit/bin

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./canvas-kit/bin
	cp ../../out/canvaskit_wasm/canvaskit.js   ./canvas-kit/bin
	cp ../../out/canvaskit_wasm/canvaskit.wasm ./canvas-kit/bin

local-example:
	rm -rf node_modules/canvas-kit
	mkdir -p node_modules
	ln -s -T ../canvas-kit  node_modules/canvas-kit
	echo "Go check out http://localhost:8000/canvas-kit/example.html"
	python serve.py
