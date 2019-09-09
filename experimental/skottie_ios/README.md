##`skottie_ios`

How to compile:

    cd $SKIA_ROOT_DIRECTORY

    cat >> BUILD.gn <<EOM
    if (is_ios && skia_use_metal) {
      group("skottie_ios") {
        deps = [ "experimental/skottie_ios" ]
      }
    }
    EOM

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
    ninja -C out/ios_arm64_mtl skottie_ios

Then install the `out/ios_arm64_mtl/skottie_ios.app` bundle.

