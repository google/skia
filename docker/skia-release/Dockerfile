# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM gcr.io/skia-public/skia-build-tools:latest

RUN cd /tmp \
  && git clone https://swiftshader.googlesource.com/SwiftShader swiftshader

# Checkout Skia.
RUN mkdir -p /tmp/skia \
  && cd /tmp/skia \
  && fetch skia

# Set fake identity for git rebase. See thread in
# https://skia-review.googlesource.com/c/buildbot/+/286537/5/docker/Dockerfile#46
RUN cd /tmp/skia/skia \
    && git config user.email "skia@skia.org" \
    && git config user.name "Skia"

# HASH must be specified.
ARG HASH
RUN if [ -z "${HASH}" ] ; then echo "HASH must be specified as a --build-arg"; exit 1; fi

RUN cd /tmp/skia/skia \
  && git fetch \
  && git reset --hard ${HASH}

# If patch ref is specified then update the ref to patch in a CL.
ARG PATCH_REF
RUN if [ ! -z "${PATCH_REF}" ] ; then cd /tmp/skia/skia \
    && git fetch https://skia.googlesource.com/skia ${PATCH_REF} \
    && git checkout FETCH_HEAD \
    && git rebase ${HASH}; fi

RUN cd /tmp/skia/skia \
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
  && chown -R skia:skia . \
  # obj is readable only by the skia user. It needs additional
  # permissions to be accessible for CI (see https://review.skia.org/487217).
  && chmod 755 -R out/Static/obj \
  # Cleanup .git directories because they are not needed and take up space.
  && find . -name .git -print0 | xargs -0 rm -rf
