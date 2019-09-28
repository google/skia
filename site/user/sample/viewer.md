Skia Viewer
==========================
The Skia Viewer displays a series of slides that exhibit specific features of Skia, including the Skia golden master images (or GMs) and programmed samples that allow interaction. In addition, the Viewer is used to debug and understand different parts of the Skia system:

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

Similarly, `--skps <skp-file-path>` will load any `.skp` files in that directory for display within the Viewer.

Other useful command-line options: using `--match <pattern>` will load only SKPs or slides matching that name; using `--slide <name>` will launch at that slide; and you can start up with a particular rendering method by using `--backend`, i.e., `--backend sw`, `--backend gl`, or `--backend vk`.

The desktop Viewers are controlled using the keyboard and mouse: left (&#x2190;) and right (&#x2192;) arrows to move from slide to slide; up (&#x2191;) and down (&#x2193;) arrows to zoom in and out; clicking and dragging will translate. Other display options and a slide picker can be found in the Tools UI, which can be toggled by hitting the spacebar.

Key                              | Action
-----------------------------|-------------
&#x2190; &#x2192; | Move between the slides
&#x2191; &#x2193; | Zoom in / out
d                 | Change render methods among raster, OpenGL and Vulkan
s                 | Display rendering times and graph
Space             | Toggle display of Tools UI

Android
-------

To build Viewer as an Android App, you will need the
[Android SDK](https://developer.android.com/studio/#command-tools) installed and your `ANDROID_HOME` environment variable set.

    mkdir ~/android-sdk
    ( cd ~/android-sdk; unzip ~/Downloads/sdk-tools-*.zip )
    yes | ~/android-sdk/tools/bin/sdkmanager --licenses
    export ANDROID_HOME=~/android-sdk  # Or wherever you installed the Android SDK.

The Viewer APK must be built by gradle which can be invoked on the command line
with the following script:

    platform_tools/android/bin/android_build_app -C <out_dir> viewer

where `<out_dir>` is the ninja out directory for android (e.g., `out/arm64`)
that you want to use to build the app. Upon completion of the script the APK
can be found at `<out_dir>/viewer.apk`

To load resources in the Android Viewer place them in
`/data/local/tmp/resources`; to load SKPs place them in `/data/local/tmp/skps`.

Swiping left and right will switch slides, pinch-zoom will zoom in and out, and
display options are available in the UI.

iOS
---

The viewer is not yet fully supported on iOS, but can be used to display
individual slides on a device by launching via `ios-deploy` with the `--match`
or `--slide` command-line options. The viewer will automatically bundle the
`resources` directory in the top-level Skia directory, and will bundle an
`skps` directory if also placed in the Skia directory.
