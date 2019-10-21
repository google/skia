# Build gcr.io/skia-public/skia-build-tools:latest, the image used to compile
# Skia in Google Cloud Build.
push-skia-build-tools:
	docker build -t skia-build-tools ./skia-build-tools
	docker tag skia-build-tools gcr.io/skia-public/skia-build-tools:latest
	docker push gcr.io/skia-public/skia-build-tools:latest