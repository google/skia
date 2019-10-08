# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM launcher.gcr.io/google/clang-debian9 AS build
RUN apt-get update && apt-get upgrade -y && apt-get install -y \
  git \
  python \
  curl \
  build-essential \
  libfontconfig-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  && groupadd -g 2000 skia \
  && useradd -u 2000 -g 2000 --home-dir /workspace/__cache skia

ADD --chown=skia:skia https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libGLESv2.so /usr/local/lib/libGLESv2.so
ADD --chown=skia:skia  https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/Latest/Linux/libEGL.so /usr/local/lib/libEGL.so
RUN cd /tmp \
  && git clone 'https://chromium.googlesource.com/chromium/tools/depot_tools.git' \
  && git clone https://swiftshader.googlesource.com/SwiftShader swiftshader \
  && mkdir -p /tmp/skia \
  && cd /tmp/skia \
  && /tmp/depot_tools/fetch skia \
  && cd skia

RUN mkdir -p /tmp/skia/skia/out/Static
RUN echo '  \n\
cc = "clang"  \n\
cxx = "clang++"  \n\
skia_use_egl = true  \n\
is_debug = false  \n\
skia_use_system_freetype2 = false  \n\
extra_cflags = [  \n\
  "-I/tmp/swiftshader/include",  \n\
  "-DGR_EGL_TRY_GLES3_THEN_GLES2",  \n\
  "-g0",  \n\
]  \n\
extra_ldflags = [  \n\
  "-L/usr/local/lib",  \n\
  "-Wl,-rpath",  \n\
  "-Wl,/usr/local/lib"  \n\
] ' > /tmp/skia/skia/out/Static/args.gn

RUN cd /tmp/skia/skia \
  && python tools/git-sync-deps \
  && ./bin/fetch-gn \
  && ./bin/gn gen out/Static \
  && git rev-parse HEAD > VERSION \
  && /tmp/depot_tools/ninja -C out/Static \
  && chown -R skia:skia .

# Uncomment the lines below and update the ref to patch in a CL.
#
#RUN cd /tmp/skia/skia \
#  && git fetch https://skia.googlesource.com/skia refs/changes/25/130325/15 \
#  && git checkout FETCH_HEAD \
#  && /tmp/depot_tools/ninja -C out/Static
