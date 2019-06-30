#! /bin/bash


OS=$1
ESY_LIBJPEG_TURBO_PREFIX=$2
if [[ $OS == 'windows' ]]
then
    cp /usr/bin/python ./python.exe
fi

python tools/git-sync-deps
ln -s third_party/externals/gyp tools/gyp
if [[ $OS == 'windows' ]]
then
    bin/gn gen $cur__target_dir/out/Static --args='is_debug=false' || exit -1
    bin/gn gen $cur__target_dir/out/Shared --args='is_debug=false is_official_build=true is_component_build=true'
else
    bin/gn gen $cur__target_dir/out/Static"--args=cc=\"clang\" cxx=\"clang++\" skia_use_system_libjpeg_turbo=true is_debug=false extra_cflags=[\"-I${ESY_LIBJPEG_TURBO_PREFIX}/include\"] extra_ldflags=[\"-L${ESY_LIBJPEG_TURBO_PREFIX}/lib\", \"-ljpeg\" ]" || exit -1
fi

ninja.exe -C $cur__target_dir/out/Static
ninja.exe -C $cur__target_dir/out/Shared

