
publish_Debian10:
	docker build -t gcc-debian10 ./Debian10/
	docker tag gcc-debian10 gcr.io/skia-public/gcc-debian10
	docker push gcr.io/skia-public/gcc-debian10

publish_Debian10-x86:
	docker build -t gcc-debian10-x86 ./Debian10-x86/
	docker tag gcc-debian10-x86 gcr.io/skia-public/gcc-debian10-x86
	docker push gcr.io/skia-public/gcc-debian10-x86

publish_Debian10-mips64el:
	docker build -t gcc-debian10-mips64el ./Debian10-mips64el/
	docker tag gcc-debian10-mips64el gcr.io/skia-public/gcc-debian10-mips64el
	docker push gcr.io/skia-public/gcc-debian10-mips64el

