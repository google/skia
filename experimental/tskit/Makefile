debug:
	npx tsc
	./compile.sh debug
	rm -rf ./npm_build/bin
	mkdir -p ./npm_build/bin
	cp ../../out/tskit_debug/tskit.js ./npm_build/bin
	cp ../../out/tskit_debug/tskit.wasm ./npm_build/bin

release:
	npx tsc
	./compile.sh
	rm -rf ./npm_build/bin
	mkdir -p ./npm_build/bin
	cp ../../out/tskit/tskit.js ./npm_build/bin
	cp ../../out/tskit/tskit.wasm ./npm_build/bin

serve:
	echo "Go check out http://localhost:8000/npm_build/example.html"
	python3 serve.py

lint:
	npx eslint . --ext .ts

apply-lint:
	npx eslint . --ext .ts --fix

generate:
	go run ./go/gen_types --input_cpp_dir ./bindings --output_namespace_dir ./bindings