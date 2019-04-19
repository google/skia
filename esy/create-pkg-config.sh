if which x86_64-w64-mingw32-gcc; then
    # TODO find out what is needed here
    platformSpecificFlags=
else
    if [ "$(uname)" == "Darwin" ]; then
        platformSpecificFlags="-framework CoreServices -framework CoreGraphics -framework CoreText -framework CoreFoundation"
    else
        # TODO find out what is needed here
        platformSpecificFlags=
    fi
fi

touch $cur__lib/skia.pc
cat >$cur__lib/skia.pc << EOF
includedir=$cur__root/include
libdir=$cur__lib

Name: skia
Description: 2D graphics library
Version: $cur__version
Cflags: -I\${includedir}/android -I\${includedir}/atlastext -I\${includedir}/c -I\${includedir}/codec -I\${includedir}/config -I\${includedir}/core -I\${includedir}/docs -I\${includedir}/effects -I\${includedir}/encode -I\${includedir}/gpu -I\${includedir}/pathops -I\${includedir}/ports -I\${includedir}/private -I\${includedir}/svg -I\${includedir}/third_party -I\${includedir}/utils -std=c++1y
Libs: -L\${libdir} $platformSpecificFlags -lskia -lstdc++
EOF