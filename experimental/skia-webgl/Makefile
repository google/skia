clean:
	rm -rf ../../out/skia_wasm
	./compile_bitcode.sh
	./compile.sh

release:
	# Doesn't clean before, to allow cached builds
	./compile_bitcode.sh
	./compile.sh
	mkdir -p ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.js   ./skia-wasm/bin
	cp ../../out/skia_wasm/skia.wasm ./skia-wasm/bin

local-example:
	rm -rf node_modules/skia-wasm
	mkdir -p node_modules
	ln -s -T ../skia-wasm  node_modules/skia-wasm
	echo "Go check out http://localhost:8000/skia-wasm/example.html"
	python serve.py