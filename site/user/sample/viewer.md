Skia Viewer
==========================
The Skia Viewer is another windowed app similar to SampleApp. Its main advantages over SampleApp are that it supports Vulkan on Linux, Windows and Android, and it uses a cleaner cross-platform windowing system (currently located in tools/viewer/sk_app). However, it is a work in progress, so it doesn't yet support all the features of SampleApp.

Like SampleApp, it displays a series of slides that exhibit specific features of the Skia system. It is mainly controlled using the keyboard: left (&#x2190;) and (&#x2192;) right arrows to move from slide to slide. Some slides require use resources stored outside the program. These resources are stored in the `<skia-path>/resources` directory.

`<skia-path>/out/Release/viewer --resourcePath <skia-path>/resources`

In addition to displaying various aspects of Skia, the Viewer is used to debug and understand different parts of the Skia system:

* Observe rendering performance - pressing the 'f' key places the Viewer in frame rate mode. It continuously draws the slide while desplaying the draw time in the title of the window.
* Try different rendering methods - pressing the 'd' key cycles among the three rendering methods: raster, OpenGL and Vulkan (on supported platforms). You can use this with the 'f' key to see the effect that the different rendering methods have on drawing performance. You can start up with a particular rendering method by using one of the command line options `--backend sw`, `--backend gl`, or `--backend vk` respectively.
* Display and manipulate your own pictures - use `--skps <skp-file-path>` to load any `.skp` files to be displayed by the Viewer. Using `--match <slide-name>` will load only SKPs or slides matching that name.

Linux, Macintosh and Windows
----------------------------

The Viewer can be built using the regular GN build process, e.g.

    bin/gn gen out/Release --args='is_debug=false'
    ninja -C out/Release viewer

Android
-------
The Viewer APK must be built by gradle which can be invoked on the command line with the following script...

    ./platform_tools/android/bin/android_build_app -C <out_dir> viewer

*   **out_dir** is the ninja out directory for android (e.g., `out/arm64`) that you want to use to
build the app

Upon completion of the script the APK can be found at <out_dir>/viewer.apk

To load SKPs in the Android viewer place them in /data/local/tmp/skps.

iOS
---
The viewer is currently not supported on iOS.

Key                              | Action
-----------------------------|-------------
&#x2190; &#x2192; | Move between the slides
&#x2191; &#x2193; | Zoom in / out
d                                   | Change render methods among raster, OpenGL and Vulkan
f                                     | Display the rendering time in the menu bar


