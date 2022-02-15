.PHONY: local
local:
	echo "Go check out http://localhost:8123/demos/hello_world/index.html"
	rm -f build
	ln -s ../modules/canvaskit/build build
	python3 -m http.server 8123
