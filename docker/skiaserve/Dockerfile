# Dockerfile for building skiaserve with SwiftShader.
FROM launcher.gcr.io/google/clang-debian9 AS build
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libEGL.so /usr/local/lib/libEGL.so
ADD args.gn /tmp/skia/out/Static/args.gn
RUN apt-get update && apt-get install -y \
  git \
  python \
  curl \
  sudo \
  build-essential \
  freeglut3-dev \
  libfontconfig-dev \
  libfreetype6-dev \
  libgif-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libpng-dev \
  libqt4-dev \
  lsb-release \
  && cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git' \
  && export PATH="${PWD}/depot_tools:${PATH}" \
  && git clone https://swiftshader.googlesource.com/SwiftShader swiftshader \
  && mkdir -p /tmp/skia \
  && cd /tmp/skia \
  && fetch skia \
  && cd skia \
  && python tools/git-sync-deps \
  && ./bin/fetch-gn \
  && gn gen out/Static \
  && ninja -C out/Static

FROM launcher.gcr.io/google/debian9
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libEGL.so /usr/local/lib/libEGL.so
RUN apt-get update && apt-get install -y \
  freeglut3 \
  libfontconfig \
  libfreetype6 \
  libglu1-mesa \
  libgif7 \
  xvfb \
  libpng16-16 \
  && rm -rf /var/lib/apt/lists/*
COPY --from=build /tmp/skia/skia/out/Static/skiaserve /bin/skiaserve
ENTRYPOINT DISPLAY=:99 /usr/bin/xvfb-run --server-args "-screen 0 1280x1024x24" /bin/skiaserve --address 0.0.0.0
