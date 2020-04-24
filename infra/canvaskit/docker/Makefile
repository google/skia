EMSDK_VERSION=1.38.47_v1

publish_canvaskit_emsdk:
	docker build -t canvaskit-emsdk ./canvaskit-emsdk/
	docker tag canvaskit-emsdk gcr.io/skia-public/canvaskit-emsdk:${EMSDK_VERSION}
	docker push gcr.io/skia-public/canvaskit-emsdk:${EMSDK_VERSION}
