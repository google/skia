build:
	./compile.sh

npm:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	#./compile.sh asm.js TODO

publish:
	cd npm-wasm; npm publish

update-major:
	cd npm-wasm; npm version major
	echo "Don't forget to publish."

update-minor:
	cd npm-wasm; npm version minor
	echo "Don't forget to publish."

update-patch:
	cd npm-wasm; npm version patch
	echo "Don't forget to publish."

dev:
	./compile.sh dev

dev-npm:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh dev
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	#./compile.sh asm.js TODO

example:
	npm install experimental-pathkit-wasm
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py