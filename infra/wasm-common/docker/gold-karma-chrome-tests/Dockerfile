# Docker container with Chrome, and karma/jasmine, to be used to run JS tests and
# collect output for Skia Infra's Gold tool (correctness checking).
#
# Tests will be run as non-root (user skia, in fact), so /OUT should have permissions
# 777 so as to be able to create output there.

FROM gcr.io/skia-public/karma-chrome-tests:77.0.3865.120_v1

COPY /tmp/gold-aggregator /opt/gold-aggregator