
---
title: "Skia Viewer"
linkTitle: "Skia Viewer"

---

The Skia Viewer displays a series of slides that exhibit specific features of Skia, including the Skia GMs and programmed samples that allow interaction. In addition, the Viewer is used to debug and understand different parts of the Skia system:

* Observe rendering performance - placing the Viewer in stats mode displays average frame times.
* Try different rendering methods - it's possible to cycle among the three rendering methods: raster, OpenGL and Vulkan (on supported platforms). You can use this with stats mode to see the effect that the different rendering methods have on drawing performance.
* Display and manipulate your own pictures.

Some slides require resources stored outside the program. These resources are stored in the `<skia-path>/resources` directory.

Linux, Macintosh and Windows
----------------------------

The Viewer can be built using the regular GN build process, e.g.

    bin/gn gen out/Release --args='is_debug=false'
    ninja -C out/Release viewer

To load resources in the desktop Viewers, use the `--resourcePath` option:

    <skia-path>/out/Release/viewer --resourcePath <skia-path>/resources

Similarly, `--skps <skp-file-path>` will load any `.skp` files in that directory
for display within the Viewer.

Other useful command-line options: using `--match <pattern>` will load only SKPs or slides
matching that name; using `--slide <name>` will launch at that slide; and you can start up
with a particular rendering method by using `--backend`, i.e., `--backend sw`, `--backend gl`,
`--backend vk`, or `--backend mtl`.

The desktop Viewers are controlled using the keyboard and mouse: left (←) and right
(→) arrows to move from slide to slide; up (↑) and down (↓) arrows to
zoom in and out; clicking and dragging will translate. Other display options and a slide
picker can be found in the Tools UI, which can be toggled by hitting the spacebar.

Key    | Action
-------|-------------
← →    | Move between the slides
↑ ↓    | Zoom in / out
d      | Change render methods among raster, OpenGL and Vulkan
s      | Display rendering times and graph
Space  | Toggle display of Tools UI

Android
-------

To build Viewer as an Android App, first follow the
[Android build instructions](/docs/user/build#android) to set up the
Android NDK and a ninja out directory. In addition, you will need the
[Android SDK](https://developer.android.com/studio/#command-tools) installed and your
`ANDROID_HOME` environment variable set.

    mkdir ~/android-sdk
    ( cd ~/android-sdk; unzip ~/Downloads/sdk-tools-*.zip )
    yes | ~/android-sdk/tools/bin/sdkmanager --licenses
    export ANDROID_HOME=~/android-sdk  # Or wherever you installed the Android SDK.

If you are not using the NDK included with the Android SDK (at ~/android-sdk/ndk-bundle
in this example) you'll need to set the environmental variable `ANDROID_NDK_HOME`, e.g.,

    export ANDROID_NDK_HOME=/tmp/ndk

The Viewer APK must be built by gradle which can be invoked on the command line
with the following script:

    platform_tools/android/bin/android_build_app -C <out_dir> viewer

where `<out_dir>` is the ninja out directory (e.g., `out/arm64`)
that you created. Upon completion of the script the APK
can be found at `<out_dir>/viewer.apk`. Install it with `adb install`.

### How to Use the App

Most app functions (except touch gestures and arrow buttons) are placed in the **left drawer**.
Click on the upper-left hamburger button to open that drawer.

#### Switch Slides

In the upper-right corner, there are two arrows: next slide, previous slide.

In the left drawer, you can directly select a slide from a list (spinner). Above that spinner,
there’s a text filter that applies to the slide list. There are hundreds of slides so if you
know the slide name, use that filter to quickly locate and show it.

#### Zoom / Translate

We support touch gestures on the slide so you can drag and pinch to zoom.

#### Change Backend

In the left drawer, you can select the backend from a list of OpenGL, Vulkan, and Raster.

#### Softkey / Stats

In the left drawer, there’s a list of softkeys. They correspond to the keyboard commands
of a desktop Viewer app. For example, you can toggle color mode or stats window. These can
be filtered like the slides.

For animation slides, we also show FPS (actually, it’s Seconds Per Frame) --- frame
refresh rate in milliseconds.

#### Loading resources / skps

TODO (https://issues.skia.org/295805469): This used to be possible with the instructions
below, but they no longer work on recent versions of Android.

To load resources in the Android Viewer place them in
`/data/local/tmp/resources`; to load SKPs place them in `/data/local/tmp/skps`.

iOS
---

Viewer on iOS is built using the regular GN process, e.g.

    bin/gn gen out/Release --args='target_os="ios" is_debug=false'
    ninja -C out/Release viewer

Like other iOS apps it can be deployed either by using something like
[ios-deploy](https://github.com/ios-control/ios-deploy)
or by building within Xcode and launching via the IDE. See the
[iOS build instructions](https://skia.org/docs/user/build#ios) for more information
on managing provisioning profiles for signing and deployment.

Viewer will
automatically bundle the `resources` directory in the top-level Skia directory,
and will bundle an `skps` directory if also placed in the Skia directory.

On iOS the Viewer provides basic touch functionality: you can view slides,
swipe between them, pinch-zoom to scale, and translate via panning. There is not
yet support for display options or selecting from a list of slides.

