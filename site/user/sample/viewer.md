Skia Viewer
==========================
The Skia Viewer displays a series of slides that exhibit specific features of the Skia system. It is mainly controlled using the keyboard: left (&#x2190;) and (&#x2192;) right arrows to move from slide to slide. Some slides require use resources stored outside the program. These resources are stored in the `<skia-path>/resources` directory.

`<skia-path>/out/Release/viewer --resourcePath <skia-path>/resources`

In addition to displaying various aspects of Skia, the Viewer is used to debug and understand different parts of the Skia system:

* Observe rendering performance - pressing the 's' key places the Viewer in stats mode, displaying a bar graph and average frame times.
* Try different rendering methods - pressing the 'd' key cycles among the three rendering methods: raster, OpenGL and Vulkan (on supported platforms). You can use this with the 's' key to see the effect that the different rendering methods have on drawing performance. You can start up with a particular rendering method by using one of the command line options `--backend sw`, `--backend gl`, or `--backend vk` respectively.
* Display and manipulate your own pictures - use `--skps <skp-file-path>` to load any `.skp` files to be displayed by the Viewer. Using `--match <pattern>` will load only SKPs or slides matching that name. Using '--slide <name>' will launch at that slide.

Other display options and a slide picker can be found in the Tools UI, which can be toggled by hitting SPACE.

Linux, Macintosh and Windows
----------------------------

The Viewer can be built using the regular GN build process, e.g.

    bin/gn gen out/Release --args='is_debug=false'
    ninja -C out/Release viewer

Android
-------
The Viewer APK must be built by gradle which can be invoked on the command line with the following script...

    ./platform_tools/android/bin/android_build_app -C <out_dir> viewer

*   <out_dir> is the ninja out directory for android (e.g., `out/arm64`) that you want to use to
build the app

Upon completion of the script the APK can be found at <out_dir>/viewer.apk

To load SKPs in the Android viewer place them in /data/local/tmp/skps.

iOS
---
The viewer is not yet fully supported on iOS, but can be used to display individual slides on a device by launching with the --match or --slide options.

Key                              | Action
-----------------------------|-------------
&#x2190; &#x2192; | Move between the slides
&#x2191; &#x2193; | Zoom in / out
d                 | Change render methods among raster, OpenGL and Vulkan
s                 | Display rendering times and graph
SPACE             | Toggle display of Tools UI
