set -ex

BASE_DIR=`cd $(dirname ${BASH_SOURCE[0]}) && pwd`
# This expects the environment variable EMSDK to be set
if [[ ! -d $EMSDK ]]; then
  echo "Be sure to set the EMSDK environment variable."
  exit 1
fi

# Navigate to SKIA_HOME from where this file is located.
pushd $BASE_DIR/../..

source $EMSDK/emsdk_env.sh
EMCC=`which emcc`
EMCXX=`which em++`

BUILD_DIR=${BUILD_DIR:="out/debugger_wasm"}
mkdir -p $BUILD_DIR

NINJA=`which ninja`

echo "Compiling bitcode"

# Inspired by https://github.com/Zubnix/skia-wasm-port/blob/master/build_bindings.sh
./bin/gn gen ${BUILD_DIR} \
  --args="cc=\"${EMCC}\" \
  cxx=\"${EMCXX}\" \
  extra_cflags_cc=[\"-frtti\"] \
  extra_cflags=[\"-s\",\"USE_FREETYPE=1\",\"-s\",\"USE_LIBPNG=1\", \"-s\", \"WARN_UNALIGNED=1\",
    \"-DSKNX_NO_SIMD\", \"-DSK_DISABLE_AAA\", \"-DSK_DISABLE_DAA\", \"-DSK_DISABLE_READBUFFER\",
    \"-DSK_DISABLE_EFFECT_DESERIALIZATION\"
  ] \
  is_debug=false \
  is_official_build=true \
  is_component_build=false \
  target_cpu=\"wasm\" \
  \
  skia_use_angle = false \
  skia_use_dng_sdk=false \
  skia_use_egl=true \
  skia_use_expat=false \
  skia_use_fontconfig=false \
  skia_use_freetype=true \
  skia_use_libheif=false \
  skia_use_libjpeg_turbo=true \
  skia_use_libpng=true \
  skia_use_libwebp=false \
  skia_use_lua=false \
  skia_use_piex=false \
  skia_use_system_libpng=true \
  skia_use_system_freetype2=true \
  skia_use_system_libjpeg_turbo = false \
  skia_use_vulkan=false \
  skia_use_zlib=true \
  skia_enable_skshaper=false \
  skia_enable_ccpr=false \
  skia_enable_nvpr=false \
  skia_enable_skpicture=false \
  skia_enable_fontmgr_empty=false \
  skia_enable_pdf=false"

# Build all the libs, we'll link the appropriate ones down below
${NINJA} -C ${BUILD_DIR} libskia.a libtool_utils.a

# export EMCC_CLOSURE_ARGS="--externs $BASE_DIR/externs.js "

echo "Generating final wasm"

# Emscripten prefers that the .a files go last in order, otherwise, it
# may drop symbols that it incorrectly thinks aren't used. One day,
# Emscripten will use LLD, which may relax this requirement.
${EMCXX} \
    -Iexperimental \
    -Iinclude/c \
    -Iinclude/codec \
    -Iinclude/config \
    -Iinclude/core \
    -Iinclude/effects \
    -Iinclude/gpu \
    -Iinclude/gpu/gl \
    -Iinclude/pathops \
    -Iinclude/private \
    -Itools/debugger \
    -Iinclude/utils/ \
    -Isrc/core/ \
    -Isrc/gpu/ \
    -Isrc/sfnt/ \
    -Isrc/shaders/ \
    -Isrc/utils/ \
    -Ithird_party/icu \
    -Itools \
    -DSK_DISABLE_READBUFFER \
    -DSK_DISABLE_AAA \
    -DSK_DISABLE_DAA \
    -std=c++14 \
    --post-js $BASE_DIR/ready.js \
    --bind \
    $BASE_DIR/debugger.cpp \
    $BUILD_DIR/libskia.a \
    $BUILD_DIR/libtool_utils.a \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORT_NAME="DebuggerInit" \
    -s FORCE_FILESYSTEM=0 \
    -s MODULARIZE=1 \
    -s NO_EXIT_RUNTIME=1 \
    -s STRICT=1 \
    -s TOTAL_MEMORY=128MB \
    -s USE_FREETYPE=1 \
    -s USE_LIBPNG=1 \
    -s WARN_UNALIGNED=1 \
    -s WASM=1 \
    -o $BUILD_DIR/debugger.js