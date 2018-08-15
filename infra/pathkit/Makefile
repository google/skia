gold-docker-image: aggregator
	# Set the build context to the current work dir, so we can copy
	# the built binary to where we need it.
	docker build -t gold-karma-chrome-tests -f ./docker/gold-karma-chrome-tests/Dockerfile .

aggregator:
	mkdir -p ./tmp
	CGO_ENABLED=0 GOOS=linux go build -o ./tmp/gold-aggregator -a ./gold/