#! /bin/bash

OS=$1
ESY_LIBJPEG_TURBO_PREFIX=$2

if [[ $OS == "windows" ]]
then
    cp /usr/bin/python $cur__target_dir/python.exe
    bin/gn gen $cur__target_dir/out/Shared --args='is_debug=false is_component_build=true'
    ninja.exe -C $cur__target_dir/out/Shared
    esy/gendef.exe - $cur__target_dir/out/Shared/skia.dll > $cur__target_dir/out/Shared/skia.def
    x86_64-W64-mingw32-dlltool.exe -D $cur__target_dir/out/Shared/skia.dll -d $cur__target_dir/out/Shared/skia.def -A -l $cur__target_dir/out/Shared/skia.a
else
    bin/gn gen $cur__target_dir/out/Static "--args=cc=\"clang\" cxx=\"clang++\" skia_use_system_libjpeg_turbo=true is_debug=false extra_cflags=[\"-I${ESY_LIBJPEG_TURBO_PREFIX}/include\"] extra_ldflags=[\"-L${ESY_LIBJPEG_TURBO_PREFIX}/lib\", \"-ljpeg\" ]" || exit -1
    ninja.exe -C $cur__target_dir/out/Static
fi
