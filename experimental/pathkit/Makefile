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

# Build the library and run the tests. If developing locally, test-continuous is better
# suited for that, although if you make changes to the C++/WASM code, you will need
# to manually call make npm-test to re-build.
test: npm-test
	npm install
	npx karma start ./karma.conf.js --single-run

test-continuous:
	echo "Assuming npm install has been run by user"
	echo "Also assuming make npm-test has also been run by a user (if needed)"
	npx karma start ./karma.conf.js --no-single-run --watch-poll

# Build the library and run the tests using the docker image.  If developing locally,
# test-docker-continuous is better, although if you make changes to the C++/WASM code,
# you will need to manually call make npm-test to re-build.
test-docker: npm-test
	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC gcr.io/skia-public/karma-chrome-tests:68.0.3440.106_v3 \
karma start /SRC/experimental/pathkit/karma-docker.conf.js --single-run

test-docker-continuous:
	echo "Assuming make npm-test has also been run by a user (if needed)"
	docker run --shm-size=2gb -v $$SKIA_ROOT:/SRC gcr.io/skia-public/karma-chrome-tests:68.0.3440.106_v3 \
karma start /SRC/experimental/pathkit/karma-docker.conf.js --no-single-run

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
	rm -rf node_modules/experimental-pathkit-wasm
	mkdir -p node_modules
	ln -s -T ../npm-wasm node_modules/experimental-pathkit-wasm
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example-test:
	rm -rf node_modules/experimental-pathkit-wasm
	mkdir -p node_modules/experimental-pathkit-wasm
	ln -s -T ../../npm-wasm/bin/test node_modules/experimental-pathkit-wasm/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py

local-example-debug:
	rm -rf node_modules/experimental-pathkit-wasm
	mkdir -p node_modules/experimental-pathkit-wasm
	ln -s -T ../../npm-wasm/bin/debug node_modules/experimental-pathkit-wasm/bin
	echo "Go check out localhost:8000/npm-wasm/example.html"
	python serve.py