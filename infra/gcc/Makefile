publish_Debian11:
	docker build -t gcc-debian11 ./Debian11/
	docker tag gcc-debian11 gcr.io/skia-public/gcc-debian11
	docker push gcr.io/skia-public/gcc-debian11

publish_Debian11-x86:
	docker build -t gcc-debian11-x86 ./Debian11-x86/
	docker tag gcc-debian11-x86 gcr.io/skia-public/gcc-debian11-x86
	docker push gcr.io/skia-public/gcc-debian11-x86
