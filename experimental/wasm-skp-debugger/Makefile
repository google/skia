debug:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./debugger/bin
	cp ../../out/debugger_wasm/debugger.js   ./debugger/bin
	cp ../../out/debugger_wasm/debugger.wasm ./debugger/bin
	# only necessary while emscripten's --preload-file is being used for testing
	cp ../../out/debugger_wasm/debugger.data ./debugger/bin

local-example:
	rm -rf node_modules/debugger
	mkdir -p node_modules
	ln -s -T ../debugger node_modules/debugger
	echo "Go check out http://localhost:8000/debugger/index.html"
	python serve.py

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make debug or release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll