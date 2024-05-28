# Dockerfile for building Skia in release mode, using 3rd party libs from DEPS, with SwiftShader.
FROM gcr.io/skia-public/skia-build-tools:latest

# Sometimes update_depot_tools fails (e.g. from a pre-goma-deletion commit).
RUN cd /tmp/depot_tools && git pull && update_depot_tools

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

# Check out skia but delete large third_party checkouts we don't actually need
# to keep the docker image smaller.
RUN cd /tmp/skia/skia \
  && gclient sync \
  && ./bin/fetch-gn \
  && ./bin/fetch-ninja \
  && rm -r third_party/externals/dawn \
           third_party/externals/icu4x \
           third_party/externals/opengl-registry \
           third_party/externals/perfetto \
           third_party/externals/swiftshader \
           third_party/externals/unicodetools

# Write args.gn.
RUN mkdir -p /tmp/skia/skia/out/Static
RUN echo '  \n\
cc = "clang"  \n\
cxx = "clang++"  \n\
skia_use_egl = true  \n\
is_debug = false  \n\
skia_enable_fontmgr_fontconfig = true \n\
skia_use_perfetto = false \n\
skia_use_libgrapheme = false \n\
skia_use_icu4x = false \n\
skia_use_system_freetype2 = false  \n\
link_pool_depth=2  \n\
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

WORKDIR /tmp/skia/skia
# Build Skia.
RUN ./bin/gn gen out/Static
RUN git rev-parse HEAD > VERSION
RUN ./bin/ninja -C out/Static
# Cleanup .git directories because they are not needed and take up space.
RUN find . -name .git -print0 | xargs -0 rm -rf
RUN chown -R skia:skia .
# obj is readable only by the skia user. It needs additional
# permissions to be accessible for CI (see https://review.skia.org/487217).
RUN chmod 755 -R out/Static/obj

FROM gcr.io/skia-public/skia-build-tools:latest
COPY --from=0 /tmp/skia/skia /tmp/skia/skia
WORKDIR /tmp/skia/skia