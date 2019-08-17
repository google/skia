#! /bin/bash

OS=$1

# Copy artifacts into output directories
if [[ $OS == 'windows' ]]
then
    cp -a $cur__target_dir/out/Shared/skia.a $cur__lib
    cp -a $cur__target_dir/out/Shared/skia.dll $cur__bin
else
    cp -a $cur__target_dir/out/Static/skia.a $cur__lib
fi

# Create pkg-config file skia.pc
if [[ $OS == "darwin" ]]
then
    platformSpecificFlags="-framework CoreServices -framework CoreGraphics -framework CoreText -framework CoreFoundation"
elif [[ $OS == "windows" ]]
then
    platformSpecificFlags="-luser32"
else
    # TODO find out what is needed here
    platformSpecificFlags=
fi

cat >$cur__lib/skia.pc << EOF
includedir=$cur__root/include
libdir=$cur__lib

Name: skia
Description: 2D graphics library
Version: $cur__version
Cflags: -I\${includedir}/android -I\${includedir}/atlastext -I\${includedir}/c -I\${includedir}/codec -I\${includedir}/config -I\${includedir}/core -I\${includedir}/docs -I\${includedir}/effects -I\${includedir}/encode -I\${includedir}/gpu -I\${includedir}/pathops -I\${includedir}/ports -I\${includedir}/private -I\${includedir}/svg -I\${includedir}/third_party -I\${includedir}/utils -std=c++1y
Libs: -L\${libdir} $platformSpecificFlags -lskia -lstdc++
EOF

if [[ $OS != "windows" ]]
then
    echo "Requires: libjpeg" >> $cur__lib/skia.pc
fi
