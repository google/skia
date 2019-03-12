build:
	./compile.sh

npm: npm-test npm-debug
	mkdir -p ./npm-wasm/bin
	mkdir -p ./npm-asmjs/bin
	./compile.sh
	cp ../../out/pathkit/pathkit.js   ./npm-wasm/bin
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin
	./compile.sh asm.js
	cp ../../out/pathkit/pathkit.js     ./npm-asmjs/bin/pathkit.js
	cp ../../out/pathkit/pathkit.js.mem ./npm-asmjs/bin/pathkit.js.mem

publish:
	cd npm-wasm; npm publish
	cd npm-asmjs; npm publish

update-major:
	cd npm-wasm; npm version major
	cd ../npm-asmjs; npm version major
	echo "Don't forget to publish."

update-minor:
	cd npm-wasm; npm version minor
	cd npm-asmjs; npm version minor
	echo "Don't forget to publish."

update-patch:
	cd npm-wasm; npm version patch
	cd npm-asmjs; npm version patch
	echo "Don't forget to publish."

# Build the library and run the tests. If developing locally, test-continuous is better
# suited for that, although if you make changes to the C++/WASM code, you will need
# to manually call make npm-test to re-build.
test: npm-test
	npm install
	npx karma start ./karma.conf.js --single-run
	ASM_JS=1 npx karma start ./karma.conf.js --single-run

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make npm-test has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

test-continuous-asmjs:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make npm-test has also been run by a user (if needed)"
	ASM_JS=1 npx karma start ./karma.conf.js --no-single-run --watch-poll

# Build the library and run the tests using the docker image.  If developing locally,
# test-docker-continuous is better, although if you make changes to the C++/WASM code,
# you will need to manually call make npm-test to re-build.
test-docker:
	mkdir -p ./npm-wasm/bin/test
	mkdir -p ./npm-asmjs/bin/test
	mkdir -p $$SKIA_ROOT/out/dockerbuild

	docker run --rm -v $$SKIA_ROOT:/SRC -v $$SKIA_ROOT/out/dockerbuild:/OUT \
gcr.io/skia-public/emsdk-release:1.38.16_v1 /SRC/infra/pathkit/build_pathkit.sh
	cp ../../out/dockerbuild/pathkit.js   ./npm-wasm/bin/test/pathkit.js
	cp ../../out/dockerbuild/pathkit.wasm ./npm-wasm/bin/test/pathkit.wasm

	docker run --rm -v $$SKIA_ROOT:/SRC -v $$SKIA_ROOT/out/dockerbuild:/OUT \
gcr.io/skia-public/emsdk-release:1.38.16_v1 /SRC/infra/pathkit/build_pathkit.sh asm.js
	cp ../../out/dockerbuild/pathkit.js     ./npm-asmjs/bin/test/pathkit.js
	cp ../../out/dockerbuild/pathkit.js.mem ./npm-asmjs/bin/test/pathkit.js.mem

	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC gcr.io/skia-public/karma-chrome-tests:72.0.3626.121_v1 \
karma start /SRC/modules/pathkit/karma.conf.js --single-run
	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC -e ASM_JS=1 gcr.io/skia-public/karma-chrome-tests:72.0.3626.121_v1 \
karma start /SRC/modules/pathkit/karma.conf.js --single-run

test-docker-continuous:
	echo "Assuming make npm-test has also been run by a user (if needed)"
	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC gcr.io/skia-public/karma-chrome-tests:72.0.3626.121_v1 \
karma start /SRC/modules/pathkit/karma.conf.js --no-single-run

test-docker-continuous-asmjs:
	echo "Assuming make npm-test has also been run by a user (if needed)"
	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC -e ASM_JS=1 gcr.io/skia-public/karma-chrome-tests:72.0.3626.121_v1 \
karma start /SRC/modules/pathkit/karma.conf.js --no-single-run

npm-test:
	# This compile time is typically faster than release and good for use with
	# local-example-test
	mkdir -p ./npm-wasm/bin/test
	mkdir -p ./npm-asmjs/bin/test
	./compile.sh test
	cp ../../out/pathkit/pathkit.js   ./npm-wasm/bin/test/pathkit.js
	cp ../../out/pathkit/pathkit.wasm ./npm-wasm/bin/test/pathkit.wasm
	./compile.sh asm.js test
	cp ../../out/pathkit/pathkit.js     ./npm-asmjs/bin/test/pathkit.js
	cp ../../out/pathkit/pathkit.js.mem ./npm-asmjs/bin/test/pathkit.js.mem

npm-debug:
	# This compile time is typically faster than release and good for use with
	# local-example-test
	mkdir -p ./npm-wasm/bin/debug
	mkdir -p ./npm-asmjs/bin/debug
	./compile.sh debug
	cp ../../out/pathkit/pathkit.js     ./npm-wasm/bin/debug/pathkit.js
	cp ../../out/pathkit/pathkit.wasm   ./npm-wasm/bin/debug/pathkit.wasm
	cp ../../out/pathkit/pathkit.wasm.map ./npm-wasm/bin/debug/pathkit.wasm.map
	./compile.sh asm.js debug
	cp ../../out/pathkit/pathkit.js     ./npm-asmjs/bin/debug/pathkit.js
	cp ../../out/pathkit/pathkit.js.map ./npm-asmjs/bin/debug/pathkit.js.map

example:
	npm install pathkit-asmjs pathkit-wasm
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example:
	rm -rf node_modules/pathkit-wasm
	rm -rf node_modules/pathkit-asmjs
	mkdir -p node_modules
	ln -s -T ../npm-wasm  node_modules/pathkit-wasm
	ln -s -T ../npm-asmjs node_modules/pathkit-asmjs
	echo "Go check out http://localhost:8000/npm-wasm/example.html"
	echo "or http://localhost:8000/npm-asmjs/example.html"
	python serve.py

local-example-test:
	rm -rf node_modules/pathkit-wasm
	rm -rf node_modules/pathkit-asmjs
	mkdir -p node_modules/pathkit-wasm
	mkdir -p node_modules/pathkit-asmjs
	ln -s -T ../../npm-wasm/bin/test node_modules/pathkit-wasm/bin
	ln -s -T ../../npm-asmjs/bin/test node_modules/pathkit-asmjs/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	echo "or http://localhost:8000/npm-asmjs/example.html"
	python serve.py

local-example-debug:
	rm -rf node_modules/pathkit-wasm
	rm -rf node_modules/pathkit-asmjs
	mkdir -p node_modules/pathkit-wasm
	mkdir -p node_modules/pathkit-asmjs
	ln -s -T ../../npm-wasm/bin/debug node_modules/pathkit-wasm/bin
	ln -s -T ../../npm-asmjs/bin/debug node_modules/pathkit-asmjs/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	echo "or http://localhost:8000/npm-asmjs/example.html"
	python serve.py

