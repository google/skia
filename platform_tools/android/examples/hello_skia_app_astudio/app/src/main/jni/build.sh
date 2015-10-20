
mkdir ./jni/skia
/Users/heidigaertner/Git/SKIA/skia/platform_tools/android/bin/android_ninja 
cp /Users/heidigaertner/Git/SKIA/skia/out/config/android-arm_v7_neon/Debug/lib/libskia_android.so ./obj/local/armeabi-v7a/libskia_android.so
cd /Users/heidigaertner/Git/SKIA/skia/platform_tools/android/examples/hello_skia_app/
/Users/heidigaertner/NVPACK/android-ndk-r10e/ndk-build NDK_DEBUG=1 V=1