clean:
	rm -rf ../../out/skia_wasm
	rm -rf ./skia-wasm/bin
	$(MAKE) release

release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.js   ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.wasm ./skia-wasm/bin

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.js   ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.wasm ./skia-wasm/bin

local-example:
	rm -rf node_modules/skia-wasm
	mkdir -p node_modules
	ln -s -T ../skia-wasm  node_modules/skia-wasm
	echo "Go check out http://localhost:8000/skia-wasm/example.html"
	python serve.py