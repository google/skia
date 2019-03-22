
ROOTDIR=$(pwd)
BUILDDIR=$cur__target_dir
INCLUDEDIR=$ROOTDIR/include

if which x86_64-w64-mingw32-gcc; then
    CC=x86_64-w64-mingw32-gcc
    # Copy runtime mingw files
    cp /usr/x86_64-w64-mingw32/sys-root/mingw/bin/*.dll $BUILDDIR/.
    ADDITIONAL_LIBS=
else
    CC=gcc
    if [ "$(uname)" == "Darwin" ]; then
        ADDITIONAL_LIBS="-framework CoreServices -framework CoreGraphics -framework CoreText -framework CoreFoundation -framework QuartzCore -framework Cocoa -framework Foundation"
    else
        ADDITIONAL_LIBS=-lm
    fi
fi

echo "Using build directory: $BUILDDIR"
echo "Root directory: $ROOTDIR"

cd $BUILDDIR
pwd

echo "Using compiler: $CC"


# Augment path to pick up libs
export PATH=$PATH:$BUILDDIR
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$BUILDDIR

# $CC \
# $ROOTDIR/tools/flags/SkCommandLineFlags.cpp \
#       "tools/gpu/GrContextFactory.cpp",
#       "tools/gpu/GrTest.cpp",
#       "tools/gpu/MemoryCache.cpp",
#       "tools/gpu/MemoryCache.h",
#       "tools/gpu/ProxyUtils.cpp",
#       "tools/gpu/TestContext.cpp",
#       "tools/gpu/atlastext/GLTestAtlasTextRenderer.cpp",
#       "tools/gpu/gl/GLTestContext.cpp",
#       "tools/gpu/gl/command_buffer/GLTestContext_command_buffer.cpp",
#       "tools/gpu/gl/null/NullGLTestContext.cpp",
#       "tools/gpu/mock/MockTestContext.cpp",
#       "tools/gpu/gl/mac/CreatePlatformGLTestContext_mac.cpp",
# $ROOTDIR/tools/sk_app/CommandSet.cpp \
# $ROOTDIR/tools/sk_app/GLWindowContext.cpp \
# $ROOTDIR/tools/sk_app/Window.cpp \
# $ROOTDIR/tools/sk_app/mac/GLWindowContext_mac.mm \
# $ROOTDIR/tools/sk_app/mac/RasterWindowContext_mac.mm \
# $ROOTDIR/tools/sk_app/mac/Window_mac.mm \
# $ROOTDIR/tools/sk_app/mac/main_mac.mm \
# $ROOTDIR/example/HelloWorld.cpp \
# -o HelloWorld.exe \
# -I$ROOTDIR/tools \
# -I$INCLUDEDIR/android \
# -I$INCLUDEDIR/atlastext \
# -I$INCLUDEDIR/c \
# -I$INCLUDEDIR/codec \
# -I$INCLUDEDIR/config \
# -I$INCLUDEDIR/core \
# -I$INCLUDEDIR/docs \
# -I$INCLUDEDIR/effects \
# -I$INCLUDEDIR/encode \
# -I$INCLUDEDIR/gpu \
# -I$INCLUDEDIR/pathops \
# -I$INCLUDEDIR/ports \
# -I$INCLUDEDIR/private \
# -I$INCLUDEDIR/svg \
# -I$INCLUDEDIR/third_party \
# -I$INCLUDEDIR/utils \
# -L$BUILDDIR \
# -lskia \
# -std=c++1y \
# -fobjc-arc \
# $ADDITIONAL_LIBS \
# --verbose

./HelloWorld.exe