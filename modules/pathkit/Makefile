BAZEL=bazelisk
WORKSPACE_DIR=../..

PATHKIT_GN_BIN_DIR=${WORKSPACE_DIR}/out/pathkit
PATHKIT_BAZEL_BIN_DIR=${WORKSPACE_DIR}/bazel-bin/modules/pathkit/pathkit

.PHONY: debug
debug:
	./compile.sh debug
	- rm -rf build/
	mkdir  -p ./build/wasm ./build/asmjs
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js     ./build/wasm/pathkit.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.wasm   ./build/wasm/pathkit.wasm

	./compile.sh asm.js debug
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js     ./build/asmjs/pathkit.js

.PHONY: release
release:
	./compile.sh
	- rm -rf build/
	mkdir  -p ./build/wasm ./build/asmjs
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js     ./build/wasm/pathkit.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.wasm   ./build/wasm/pathkit.wasm

	./compile.sh asm.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js     ./build/asmjs/pathkit.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js.mem ./build/asmjs/pathkit.js.mem

.PHONY: debug-bazel
debug-bazel:
	${BAZEL} build :pathkit --config=ck_full_webgl2_debug
	- rm -rf build/
	mkdir  -p ./build/wasm ./build/asmjs
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.js     ./build/wasm/pathkit.js
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.wasm   ./build/wasm/pathkit.wasm
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.js     ./build/asmjs/pathkit.js

.PHONY: npm
npm:
	rm -rf ./npm-wasm/bin ./npm-asmjs/bin
	mkdir -p ./npm-wasm/bin ./npm-asmjs/bin
	./compile.sh
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js   ./npm-wasm/bin
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.wasm ./npm-wasm/bin
	./compile.sh asm.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js     ./npm-asmjs/bin/pathkit.js
	cp ${PATHKIT_GN_BIN_DIR}/pathkit.js.mem ./npm-asmjs/bin/pathkit.js.mem
	cp CHANGELOG.md ./npm-wasm
	cp CHANGELOG.md ./npm-asmjs

.PHONY: npm-bazel
npm-bazel:
	rm -rf ./npm-wasm/bin ./npm-asmjs/bin
	mkdir -p ./npm-wasm/bin ./npm-asmjs/bin
	cd ${WORKSPACE_DIR} && \
		${BAZEL} build //modules/pathkit:pathkit --config=ck_full_webgl2_release
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.js     ./npm-wasm/bin
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.wasm   ./npm-wasm/bin
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.js     ./npm-asmjs/bin
	cp ${PATHKIT_BAZEL_BIN_DIR}/pathkit.js.mem ./npm-asmjs/bin
	cp CHANGELOG.md ./npm-wasm
	cp CHANGELOG.md ./npm-asmjs

.PHONY: test-continuous
test-continuous:
	echo "Assuming npm ci has been run by user"
	echo "Also assuming make debug or make release has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

.PHONY: test-continuous-asmjs
test-continuous-asmjs:
	echo "Assuming npm ci has been run by user"
	echo "Also assuming make debug or make release has also been run by a user (if needed)"
	ASM_JS=1 npx karma start ./karma.conf.js --no-single-run --watch-poll

.PHONY: local-example
local-example:
	echo "Go check out http://localhost:8000/npm-wasm/example.html"
	echo "or http://localhost:8000/npm-asmjs/example.html"
	python3 ${WORKSPACE_DIR}/tools/serve_wasm.py
