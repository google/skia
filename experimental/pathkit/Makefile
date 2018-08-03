build:
	./compile.sh

npm:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	#./compile.sh asm.js TODO

	mkdir -p ./npm-wasm/bin/debug
	mkdir -p ./npm-asmjs/bin/debug
	./compile.sh dev
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin/debug/pathkit.js
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin/debug/pathkit.wasm
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

example:
	npm install experimental-pathkit-wasm
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example:
	rm -rf node_modules
	mkdir -p node_modules
	ln -s -T ../npm-wasm node_modules/experimental-pathkit-wasm
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example-debug:
	rm -rf node_modules
	mkdir -p node_modules/experimental-pathkit-wasm
	ln -s -T ../../npm-wasm/bin/debug node_modules/experimental-pathkit-wasm/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py