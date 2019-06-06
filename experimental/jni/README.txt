Build and run instructions:

1. Build Skia library for android as a shared library. For example:

bin/gn gen out/arm64-v8a --args='is_debug=false ndk="/usr/local/google/home/stani/Android/Sdk/ndk-bundle" target_cpu="arm64" ndk_api=28 is_component_build=true'
ninja -C out/arm64-v8a :skia

2. Copy Skia libskia.so file from skia output folder (for example "SKIA_ROOT/out/arm64-v8a/libskia.so") to SKIA_ROOT/experimental/jni/skiajni/libs/arm64-v8a" folder

3. Use Android Studio to open and build the 2 projects in "SKIA_ROOT/experimental/jni/":
- an Android library "skiajni" that exposes Skia Java API with bindings that cover 100% of Skia C API
- a demo Android app, which consumes the Skia Java API to draw into a PNG file and stores the output on phone.
The demo app has a feature to regenerate JNI source code (Java and C++) based on Skia IDL definition from
"SKIA_ROOT/experimental/jni/skiajni/src/main/cpp/generator/skia_api_idl.cpp".

4. "RUN TEST" button renders in a PNG file. Result can be downloaded from the phone with
"adb pull /sdcard/skia-java-example.png"

5. "GENERATE JNI BINDINGS" button generates JNI bindings in "/sdcard/jni".
Use "adb pull /sdcard/jni" to pull the result from the phone.
Replace Java files in SKIA_ROOT/experimental/jni/skiajni/src/main/java/org/skia"
Replace C++ file in "SKIA_ROOT/experimental/jni/skiajni/src/main/cpp/org.skia_jni.cpp"

