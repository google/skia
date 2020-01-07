FROM gcr.io/skia-public/base-cipd:testing-slim

USER root

RUN apt-get update && apt-get upgrade -y && apt-get install -y  \
  ca-certificates \
  clang \
  libfontconfig-dev \
  libglu-dev \
  python \
  && rm -rf /var/lib/apt/lists/*

RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git /opt/depot_tools

ENV PATH="/opt/depot_tools:${PATH}"