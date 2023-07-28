
publish_Debian10:
	docker build -t gcc-debian10 ./Debian10/
	docker tag gcc-debian10 gcr.io/skia-public/gcc-debian10
	docker push gcr.io/skia-public/gcc-debian10

publish_Debian10-x86:
	docker build -t gcc-debian10-x86 ./Debian10-x86/
	docker tag gcc-debian10-x86 gcr.io/skia-public/gcc-debian10-x86
	docker push gcr.io/skia-public/gcc-debian10-x86

publish_Debian11:
	docker build -t gcc-debian11 ./Debian11/
	docker tag gcc-debian11 gcr.io/skia-public/gcc-debian11
	docker push gcr.io/skia-public/gcc-debian11

publish_Debian11-x86:
	docker build -t gcc-debian11-x86 ./Debian11-x86/
	docker tag gcc-debian11-x86 gcr.io/skia-public/gcc-debian11-x86
	docker push gcr.io/skia-public/gcc-debian11-x86
