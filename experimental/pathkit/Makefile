build:
	./compile.sh

npm:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin

	mkdir -p ./npm-wasm/bin/test
	mkdir -p ./npm-asmjs/bin/test
	./compile.sh test
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin/test/pathkit.js
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin/test/pathkit.wasm

	mkdir -p ./npm-wasm/bin/debug
	mkdir -p ./npm-asmjs/bin/debug
	./compile.sh debug
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

test:
	./compile.sh test

npm-test:
	# This compile time is typically faster than release and good for use with
	# local-example-test
	mkdir -p ./npm-wasm/bin/test
	mkdir -p ./npm-asmjs/bin/test
	./compile.sh test
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin/test/pathkit.js
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin/test/pathkit.wasm

npm-debug:
	# This compile time is typically faster than release and good for use with
	# local-example-test
	mkdir -p ./npm-wasm/bin/debug
	mkdir -p ./npm-asmjs/bin/debug
	./compile.sh debug
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin/debug/pathkit.js
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin/debug/pathkit.wasm

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

local-example-test:
	rm -rf node_modules
	mkdir -p node_modules/experimental-pathkit-wasm
	ln -s -T ../../npm-wasm/bin/test node_modules/experimental-pathkit-wasm/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example-debug:
	rm -rf node_modules
	mkdir -p node_modules/experimental-pathkit-wasm
	ln -s -T ../../npm-wasm/bin/debug node_modules/experimental-pathkit-wasm/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py