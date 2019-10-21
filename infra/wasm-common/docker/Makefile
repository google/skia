EMSDK_VERSION=1.38.47_v1

# Can check CHROME_VERSION with
# docker run karma-chrome-tests /usr/bin/google-chrome-stable --version
CHROME_VERSION=72.0.3626.121_v1

publish_emsdk_base:
	docker build --no-cache -t emsdk-base ./emsdk-base/
	docker tag emsdk-base gcr.io/skia-public/emsdk-base:${EMSDK_VERSION}
	docker push gcr.io/skia-public/emsdk-base:${EMSDK_VERSION}
	docker tag emsdk-base gcr.io/skia-public/emsdk-base:prod
	docker push gcr.io/skia-public/emsdk-base:prod

publish_karma_chrome_tests:
	docker build --no-cache -t karma-chrome-tests ./karma-chrome-tests/
	docker tag karma-chrome-tests gcr.io/skia-public/karma-chrome-tests:${CHROME_VERSION}
	docker push gcr.io/skia-public/karma-chrome-tests:${CHROME_VERSION}

