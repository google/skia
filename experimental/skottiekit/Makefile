debug: clean_examples
	# Does an incremental build where possible.
	./compile.sh debug
	cp ../../out/skottiekit_debug/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm     ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm.map ./examples/bin

debug_cpu: clean_examples
	# Does an incremental build where possible.
	./compile.sh debug cpu
	cp ../../out/skottiekit_debug/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm     ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm.map ./examples/bin

release: clean_examples
	# Does an incremental build where possible.
	./compile.sh
	cp ../../out/skottiekit/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit/skottiekit.wasm     ./examples/bin

release_cpu: clean_examples
	# Does an incremental build where possible.
	./compile.sh cpu
	cp ../../out/skottiekit/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit/skottiekit.wasm     ./examples/bin

release_full: clean_examples
	# Does an incremental build where possible.
	./compile.sh full-build
	cp ../../out/skottiekit/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit/skottiekit.wasm     ./examples/bin

npm: clean_npm
	./compile.sh full-build
	cp ../../out/skottiekit/skottiekit.js       ./skottiekit/bin
	cp ../../out/skottiekit/skottiekit.wasm     ./skottiekit/bin

clean_examples:
	rm -f ./examples/bin/*
	mkdir -p ./examples/bin

clean_npm:
	rm -f ./skottiekit/bin/*
	mkdir -p ./skottiekit/bin

serve:
	echo "Go check out http://localhost:8001/"
	cd examples && python3 python3 ../../../tools/serve_wasm.py
