# The initial build is derived from the same base image as fiddler-base, to
# ensure that we end up with the same package versions.
FROM debian:testing-slim@sha256:0aff819e18d66aa7e3795a5145494f7bfbf55df4710494fa9746c2df1b51231b AS skia-build

# Use a package archive to install packages at frozen versions. This is the
# same archive as the one used for fiddler-base.
RUN apt-get update && apt-get -y install ca-certificates \
  && echo 'Package: *\nPin: origin snapshot-cloudflare.debian.org\nPin-Priority: 1001' > /etc/apt/preferences.d/snapshot \
  && echo 'deb [check-valid-until=no] https://snapshot-cloudflare.debian.org/archive/debian/20251119T143347Z trixie main' > /etc/apt/sources.list \
  && echo 'Acquire::https::snapshot-cloudflare.debian.org::Verify-Peer "false";' > /etc/apt/apt.conf.d/99snapshot \
  && apt-get -o Acquire::https::Verify-Peer=false update \
  && apt-get -y install --allow-downgrades \
    bash \
    build-essential \
    ca-certificates \
    clang \
    curl \
    dash \
    ffmpeg \
    file \
    git \
    libfontconfig-dev \
    libfontconfig1 \
    libgl1-mesa-dev \
    libglu1-mesa \
    libglu1-mesa-dev \
    nano \
    procps \
    python3 \
    xvfb \
    ninja-build=1.12.1-1

# We need to set the .gclient config so gclient sync works later to get our
# third party deps. We want `"managed": False` to make gclient not sync Skia and
# instead use the files we copied into the container. We use "custom_deps" to
# avoid syncing large third-party deps that we don't need for fiddler.
RUN cd /tmp \
  && git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git \
  && echo 'solutions = [{\n\
  "name"        : "skia",\n\
  "url"         : "https://skia.googlesource.com/skia.git",\n\
  "deps_file"   : "DEPS",\n\
  "managed"     : False,\n\
  "custom_deps" : {\n\
    "skia/bin": None,\n\
    "skia/third_party/externals/dawn": None,\n\
    "skia/third_party/externals/emsdk": None,\n\
    "skia/third_party/externals/icu4x": None,\n\
    "skia/third_party/externals/opengl-registry": None,\n\
    "skia/third_party/externals/perfetto": None,\n\
    "skia/third_party/externals/swiftshader": None,\n\
    "skia/third_party/externals/unicodetools": None,\n\
  },\n\
  "custom_vars": {},\n\
}]\n\
' > /tmp/.gclient

# https://stackoverflow.com/a/44766666
# Copy the skia files that Louhi has cloned into the image. Louhi hasn't synced third party
# deps, so we will need to do that before compiling (see next steps).
COPY . /tmp/skia

RUN cd /tmp/skia \
  && ./bin/fetch-gn \
  && PATH="/tmp/depot_tools:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" gclient sync --no-history

WORKDIR /tmp/skia/

RUN rm -rf ./out \
  && mkdir -p ./out/Static

RUN echo '  \n\
cc = "clang"  \n\
cxx = "clang++"  \n\
skia_use_egl = true  \n\
is_debug = false  \n\
skia_enable_fontmgr_fontconfig = true \n\
skia_use_perfetto = false \n\
skia_use_libgrapheme = false \n\
skia_use_icu4x = false \n\
skia_use_partition_alloc = false \n\
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
] ' > ./out/Static/args.gn

# Build Skia once so that incremental builds are much faster
RUN ./bin/gn gen out/Static
RUN git rev-parse HEAD > VERSION
RUN ninja -C out/Static fiddle

# Cleanup .git directories because they are not needed and take up space.
# (can't do this sooner because we need to create VERSION)
RUN find . -name .git -print0 | xargs -0 rm -rf

# Copy the resulting Skia repo and build outputs into the fiddler-base image.
# We wait as late as possible to avoid expensive rebuilds while experimenting
# with changes to fiddler-base.
FROM gcr.io/skia-public/fiddler-base@sha256:0231316fe50449fbf9971a9e891e4c4d37db0adbba596f9591a20eaa215a298e
COPY --from=skia-build --chown=skia:skia /tmp/skia /tmp/skia
