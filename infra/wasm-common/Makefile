gold_docker_image: aggregator
	# Set the build context to the current work dir, so we can copy
	# the built binary to where we need it.
	docker build -t gold-karma-chrome-tests -f ./docker/gold-karma-chrome-tests/Dockerfile .

perf_docker_image: aggregator
	# Set the build context to the current work dir, so we can copy
	# the built binary to where we need it.
	docker build -t perf-karma-chrome-tests -f ./docker/perf-karma-chrome-tests/Dockerfile .

aggregator:
	mkdir -p ./tmp
	CGO_ENABLED=0 GOOS=linux go build -o ./tmp/gold-aggregator -a ./gold/
	mkdir -p ./tmp
	CGO_ENABLED=0 GOOS=linux go build -o ./tmp/perf-aggregator -a ./perf/

# Can check CHROME_VERSION with
# docker run karma-chrome-tests /usr/bin/google-chrome-stable --version
CHROME_VERSION=77.0.3865.120_v2

publish_gold_karma_chrome_tests: gold_docker_image
	docker tag gold-karma-chrome-tests gcr.io/skia-public/gold-karma-chrome-tests:${CHROME_VERSION}
	docker push gcr.io/skia-public/gold-karma-chrome-tests:${CHROME_VERSION}

publish_perf_karma_chrome_tests: perf_docker_image
	docker tag perf-karma-chrome-tests gcr.io/skia-public/perf-karma-chrome-tests:${CHROME_VERSION}
	docker push gcr.io/skia-public/perf-karma-chrome-tests:${CHROME_VERSION}
