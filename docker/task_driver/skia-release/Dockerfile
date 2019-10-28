# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM gcr.io/skia-public/skia-build-tools:latest

RUN git clone https://swiftshader.googlesource.com/SwiftShader /tmp/swiftshader

# Checkout Skia.
RUN ./bin/fetch-gn

# Write args.gn.
RUN mkdir -p out/Static
RUN cp docker/task_driver/skia-release/args.gn out/Static/args.gn

# Build Skia.
RUN ./bin/gn gen out/Static \
  && git rev-parse HEAD > VERSION \
  && ninja -C out/Static \
  && chown -R skia:skia .

# TODO
#  * td copy Dockerfile to root of checkout.
#  * td "docker build ."
