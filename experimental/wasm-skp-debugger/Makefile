release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./debugger/bin
	cp ../../out/debugger_wasm/debugger.js   ./debugger/bin
	cp ../../out/debugger_wasm/debugger.wasm ./debugger/bin
	echo "const SKIA_VERSION = 'local release build';" > ./debugger/bin/version.js

debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./debugger/bin
	cp ../../out/debugger_wasm_debug/debugger.js   ./debugger/bin
	cp ../../out/debugger_wasm_debug/debugger.wasm ./debugger/bin
	echo "const SKIA_VERSION = 'local debug build';" > ./debugger/bin/version.js

# When testing the debugger locally, these assets need to be copied to it's build dir.
move-assets:
	rm -rf ${SKIA_INFRA_ROOT}/debugger-app/build/debugger/
	mkdir ${SKIA_INFRA_ROOT}/debugger-app/build/debugger/
	cp ./debugger/bin/debugger.js   ${SKIA_INFRA_ROOT}/debugger-app/build/debugger/
	cp ./debugger/bin/debugger.wasm ${SKIA_INFRA_ROOT}/debugger-app/build/debugger/
	cp ./debugger/bin/version.js ${SKIA_INFRA_ROOT}/debugger-app/build/

# See buildbot/debugger-app/ for running the frontend.
local-debug: debug move-assets

local-release: release move-assets

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make debug or release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll
