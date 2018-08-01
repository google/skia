.PHONY: npm-package
npm-package:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	#./compile.sh asm.js


.PHONY: dev
dev:
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh dev
	cp ../../out/pathkit/pathkit.js ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	#./compile.sh asm.js

