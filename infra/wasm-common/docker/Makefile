# Can check CHROME_VERSION with
# docker run karma-chrome-tests /usr/bin/google-chrome-stable --version
CHROME_VERSION=87.0.4280.88_v1

publish_karma_chrome_tests:
	docker build --no-cache -t karma-chrome-tests ./karma-chrome-tests/
	docker tag karma-chrome-tests gcr.io/skia-public/karma-chrome-tests:${CHROME_VERSION}
	docker push gcr.io/skia-public/karma-chrome-tests:${CHROME_VERSION}

