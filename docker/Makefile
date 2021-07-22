# Build gcr.io/skia-public/skia-build-tools:latest, the image used to compile
# Skia in Google Cloud Build.
push-skia-build-tools:
	docker build -t skia-build-tools ./skia-build-tools
	docker tag skia-build-tools gcr.io/skia-public/skia-build-tools:latest
	docker push gcr.io/skia-public/skia-build-tools:latest

CMAKE_VERSION=3.13.4_v4

push-cmake-release:
	docker build --no-cache --tag cmake-release ./cmake-release
	docker tag cmake-release gcr.io/skia-public/cmake-release:${CMAKE_VERSION}
	docker push gcr.io/skia-public/cmake-release:${CMAKE_VERSION}