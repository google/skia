Animation Player
================

Skia now offers a performant, secure native player for JSON animations derived
from the Bodymovin plugin for After Effects. It can be used on any platform
where you are using Skia, including Android & iOS.

The player aims to build upon the Lottie player widely used for animations
today, improving on the performance, feature set, and platform cohesiveness for
our clients. We are big fans of the Bodymovin format and where possible,
contributing advancements back to Bodymovin/Lottie.

<br>

Sample JSON animations
----------------------

Here are some test samples rendering with Skia's animation player:

<a href="https://skottie.skia.org/e6741dda67629da1f80c254dad3df865">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/e6741dda67629da1f80c254dad3df865" width=200 height=200></skottie-inline-sk>
</a>

*Sample animations courtesy of the lottiefiles.com community

<br>

Test server
-----------

Test your Lottie files in our player at https://skottie.skia.org

<br>

The code
--------
Skia's animation code entry point can be found here on
[Googlesource](https://skia.googlesource.com/skia/+/master/modules/skottie/include/Skottie.h)
and [GitHub](https://github.com/google/skia/blob/master/modules/skottie/include/Skottie.h).
The code is part of Skia's library but can also be made available as a separate
package.

<br>

Embedding examples
------------------
Sample C code for using the Skottie native player can be found
[here](https://github.com/google/skia/blob/master/modules/skottie/src/SkottieTool.cpp).

Android app code for inspiration can be found 
[here](https://github.com/google/skia/tree/master/platform_tools/android/apps/skottie).

Example code embedding Skottie into our Viewer app is
[here](https://github.com/google/skia/blob/master/tools/viewer/SkottieSlide.cpp).

The Viewer or Skottie Android apps can be built following [these](https://skia.org/user/sample/viewer)
instructions.
