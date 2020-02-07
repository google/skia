# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM gcr.io/skia-public/skia-build-tools:latest

RUN cd /tmp \
  && git clone https://swiftshader.googlesource.com/SwiftShader swiftshader

# Checkout Skia.
RUN mkdir -p /tmp/skia \
  && cd /tmp/skia \
  && fetch skia \
  && cd skia \
  && gclient sync \
  && ./bin/fetch-gn

# Write args.gn.
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

# Build Skia.
RUN cd /tmp/skia/skia \
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
