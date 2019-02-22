EMSDK_VERSION=1.38.27_v1

publish_canvaskit_emsdk:
	docker build -t canvaskit-emsdk ./canvaskit-emsdk/
	docker tag emsdk-base gcr.io/skia-public/canvaskit-emsdk:${EMSDK_VERSION}
	docker push gcr.io/skia-public/canvaskit-emsdk:${EMSDK_VERSION}