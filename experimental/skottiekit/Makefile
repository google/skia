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

clean_examples:
	rm -f ./examples/bin/*
	mkdir -p ./examples/bin

serve:
	echo "Go check out http://localhost:8001/"
	cd examples && python ../serve.py