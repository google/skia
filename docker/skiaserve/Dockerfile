# Dockerfile for building skiaserve with SwiftShader.
FROM launcher.gcr.io/google/clang-debian9 AS build
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libEGL.so /usr/local/lib/libEGL.so
RUN mkdir -p /tmp/skia/out/Static
RUN echo '  \n\
cc = clang  \n\
cxx = clang++  \n\
skia_use_egl = true  \n\
is_debug = false  \n\
skia_use_system_freetype2 = false  \n\
extra_cflags = [  \n\
  "-I/tmp/swiftshader/include",  \n\
  "-DGR_EGL_TRY_GLES3_THEN_GLES2",  \n\
]  \n\
extra_ldflags = [  \n\
  "-L/usr/local/lib",  \n\
  "-Wl,-rpath",  \n\
  "-Wl,/usr/local/lib"  \n\
] ' > /tmp/skia/out/Static/args.gn
RUN apt-get update && apt-get install -y \
  git \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libfreetype6-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
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
  libfontconfig \
  libglu1-mesa \
  xvfb \
  && rm -rf /var/lib/apt/lists/*
COPY --from=build /tmp/skia/skia/out/Static/skiaserve /bin/skiaserve
ENTRYPOINT DISPLAY=:99 /usr/bin/xvfb-run --server-args "-screen 0 1280x1024x24" /bin/skiaserve --address 0.0.0.0
