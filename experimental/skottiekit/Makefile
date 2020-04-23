debug:
	# Does an incremental build where possible.
	./compile.sh debug
	mkdir -p ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm     ./examples/bin
	cp ../../out/skottiekit_debug/skottiekit.wasm.map ./examples/bin

release:
	# Does an incremental build where possible.
	./compile.sh
	mkdir -p ./examples/bin
	cp ../../out/skottiekit/skottiekit.js       ./examples/bin
	cp ../../out/skottiekit/skottiekit.wasm     ./examples/bin

serve:
	echo "Go check out http://localhost:8001/"
	cd examples && python ../serve.py