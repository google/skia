
ROOTDIR=$(pwd)
BUILDDIR=$cur__target_dir
INCLUDEDIR=$ROOTDIR/include

if which x86_64-w64-mingw32-gcc; then
    CC=x86_64-w64-mingw32-gcc
    # Copy runtime mingw files
    cp /usr/x86_64-w64-mingw32/sys-root/mingw/bin/*.dll $BUILDDIR/.
    export PKG_CONFIG_PATH=$cur__lib;$PKG_CONFIG_PATH
else
    CC=gcc
    export PKG_CONFIG_PATH=$cur__lib:$PKG_CONFIG_PATH
    # if [ "$(uname)" == "Darwin" ]; then
    # else
    # fi
fi

echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
echo "Using build directory: $BUILDDIR"
echo "Root directory: $ROOTDIR"

cd $BUILDDIR
pwd

echo "Using compiler: $CC"


# Augment path to pick up libs
export PATH=$PATH:$BUILDDIR
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$BUILDDIR

$CC \
$ROOTDIR/esy/test.cpp \
-o test.exe \
`pkg-config --cflags --libs skia` \
--verbose

./test.exe $BUILDDIR/test.png