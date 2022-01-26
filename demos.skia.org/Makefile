.PHONY: local
local:
	echo "Go check out http://localhost:8123/demos/hello_world/index.html"
	rm -f build
	ln -s ../modules/canvaskit/build build
	python2 -m SimpleHTTPServer 8123
