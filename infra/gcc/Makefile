
publish_Debian10:
	docker build -t gcc-debian10 ./Debian10/
	docker tag gcc-debian10 gcr.io/skia-public/gcc-debian10
	docker push gcr.io/skia-public/gcc-debian10
