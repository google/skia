# Skottie iOS Example App

## Metal

How to compile for the Metal backend:

    cd $SKIA_ROOT_DIRECTORY

    mkdir -p out/ios_arm64_mtl
    cat > out/ios_arm64_mtl/args.gn <<EOM
    target_os="ios"
    target_cpu="arm64"
    skia_use_metal=true
    skia_use_expat=false
    skia_enable_pdf=false
    EOM

    tools/git-sync-deps
    bin/gn gen out/ios_arm64_mtl
    ninja -C out/ios_arm64_mtl skottie_example

Then install the `out/ios_arm64_mtl/skottie_example.app` bundle.

## CPU

How to compile for the CPU backend:

    cd $SKIA_ROOT_DIRECTORY

    mkdir -p out/ios_arm64_cpu
    cat > out/ios_arm64_cpu/args.gn <<EOM
    target_cpu="arm64"
    target_os="ios"
    skia_enable_gpu=false
    skia_enable_pdf=false
    skia_use_expat=false
    EOM

    tools/git-sync-deps
    bin/gn gen out/ios_arm64_cpu
    ninja -C out/ios_arm64_cpu skottie_example

Then install the `out/ios_arm64_cpu/skottie_example.app` bundle.

## OpenGL

How to compile for the OpenGL backend:

    cd $SKIA_ROOT_DIRECTORY

    mkdir -p out/ios_arm64_gl
    cat > out/ios_arm64_gl/args.gn <<EOM
    target_cpu="arm64"
    target_os="ios"
    skia_enable_gpu=true
    skia_use_metal=false
    skia_enable_pdf=false
    skia_use_expat=false
    EOM

    tools/git-sync-deps
    bin/gn gen out/ios_arm64_gl
    ninja -C out/ios_arm64_gl skottie_example

Then install the `out/ios_arm64_gl/skottie_example.app` bundle.
