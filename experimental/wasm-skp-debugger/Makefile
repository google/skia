release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./debugger/bin
	cp ../../out/debugger_wasm/debugger.js   ./debugger/bin
	cp ../../out/debugger_wasm/debugger.wasm ./debugger/bin

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./debugger/bin
	cp ../../out/debugger_wasm_debug/debugger.js   ./debugger/bin
	cp ../../out/debugger_wasm_debug/debugger.wasm ./debugger/bin

move-assets:
	# assume the developer also has a checkout of buildbot here and wants to test the newly
	# built wasm debugger with the interface.
	cp ./debugger/bin/debugger.js   ${SKIA_INFRA_ROOT}/debugger-assets/res/js
	cp ./debugger/bin/debugger.wasm ${SKIA_INFRA_ROOT}/debugger-assets/res

# A barebones local example of the wasm module included in a page.
# for the real interface see
# https://github.com/google/skia-buildbot/blob/master/debugger-assets/res/imp/wasm-app.html
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
