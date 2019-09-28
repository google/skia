# Docker container with Google Chrome and puppeteer; to be used to
# collect output for Skia Infra's Gold tool (correctness checking).
#
# Tests will be run as non-root (user skia, in fact), so /OUT should have permissions
# 777 so as to be able to create output there.

FROM gcr.io/skia-public/lottie-web-puppeteer:v2

COPY /tmp/gold-aggregator /opt/gold-aggregator