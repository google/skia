FROM gcr.io/skia-public/fiddler-base@sha256:02e3ab2d1e4e0888b788b3adc6d6b955f283e73e0245a9f02552a6d654f33c4c

# https://stackoverflow.com/a/44766666
# Copy the skia files that Louhi has cloned into the image. Louhi hasn't synced third party
# deps, so we will need to do that before compiling (see next steps).
COPY --chown=skia:skia . /tmp/skia

# We need to call gclient config so gclient sync works later to get our third party deps
# We want --unmanaged to make gclient not sync Skia and instead use the files we copied
# into the container.
RUN cd /tmp && gclient config --unmanaged --name=skia https://skia.googlesource.com/skia.git

WORKDIR /tmp/skia/

# Fetch things we need but delete large third-party folders that we don't need for fiddler
RUN ./bin/fetch-gn \
  && ./bin/fetch-ninja \
  && gclient sync \
  && rm -rf third_party/externals/dawn \
           third_party/externals/emsdk \
           third_party/externals/icu4x \
           third_party/externals/opengl-registry \
           third_party/externals/perfetto \
           third_party/externals/swiftshader \
           third_party/externals/unicodetools

RUN mkdir -p ./out/Static
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
] ' > ./out/Static/args.gn

# Build Skia once so that incremental builds are much faster
RUN ./bin/gn gen out/Static
RUN git rev-parse HEAD > VERSION
RUN ./third_party/ninja/ninja -C out/Static fiddle

# Cleanup .git directories because they are not needed and take up space.
# (can't do this sooner because we need to create VERSION)
RUN find . -name .git -print0 | xargs -0 rm -rf
