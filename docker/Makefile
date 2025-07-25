CMAKE_VERSION=3.29.0_v1

push-cmake-release:
	#docker build --no-cache --tag cmake-release ./cmake-release
	docker tag cmake-release gcr.io/skia-public/cmake-release:${CMAKE_VERSION}
	docker push gcr.io/skia-public/cmake-release:${CMAKE_VERSION}
