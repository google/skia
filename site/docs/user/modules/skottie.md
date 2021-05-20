---
title: 'Skottie - Lottie Animation Player'
linkTitle: 'Skottie - Lottie Animation Player'

weight: 10
---

Skia now offers a performant, secure native player for JSON animations derived
from the Bodymovin plugin for After Effects. It can be used on any platform
where you are using Skia, including Android & iOS.

The player aims to build upon the Lottie player widely used for animations
today, improving on the performance, feature set, and platform cohesiveness for
our clients. We are big fans of the Bodymovin format and where possible,
contributing advancements back to Bodymovin/Lottie.

<br>

## Sample JSON animations

Here are some test samples rendering with Skia's animation player:

<a href="https://skottie.skia.org/e6741dda67629da1f80c254dad3df865">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/e6741dda67629da1f80c254dad3df865" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/ffea72cf6be48fa061671c124ed7789c">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/ffea72cf6be48fa061671c124ed7789c" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/00e850cdbed7304985eaefe98a4e8a9c">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/00e850cdbed7304985eaefe98a4e8a9c" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/e1aca009d5ebec9bd122b87b018bb673">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/e1aca009d5ebec9bd122b87b018bb673" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/821fd79dd7437b97ba891e7a00970a06">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/821fd79dd7437b97ba891e7a00970a06" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/ad63f250084685c96edd9b52ae2f436b">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/ad63f250084685c96edd9b52ae2f436b" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/40f78ddc751c16348a08e1d61d3e78b1">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/40f78ddc751c16348a08e1d61d3e78b1" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/fc42db7c75741437b5cb0e90b3febc65">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/fc42db7c75741437b5cb0e90b3febc65" width=200 height=200></skottie-inline-sk>
</a>
<a href="https://skottie.skia.org/c16eee287f2cea44102b6670c66e60ab">
  <skottie-inline-sk src="https://skottie.skia.org/_/j/c16eee287f2cea44102b6670c66e60ab" width=200 height=200></skottie-inline-sk>
</a>

\*Sample animations courtesy of the lottiefiles.com community

<br>

## Test server

Test your Lottie files in our player at https://skottie.skia.org

<br>

## The code

Skia's animation code entry point can be found here on
[Googlesource](https://skia.googlesource.com/skia/+/main/modules/skottie/include/Skottie.h)
and
[GitHub](https://github.com/google/skia/blob/master/modules/skottie/include/Skottie.h).
The code is part of Skia's library but can also be made available as a separate
package.

<br>

## Embedding examples

Sample C code for using the Skottie native player can be found
[here](https://github.com/google/skia/blob/master/modules/skottie/src/SkottieTool.cpp).

Android app code for inspiration can be found
[here](https://github.com/google/skia/tree/master/platform_tools/android/apps/skottie).

Example code embedding Skottie into our Viewer app is
[here](https://github.com/google/skia/blob/master/tools/viewer/SkottieSlide.cpp).

The Viewer or Skottie Android apps can be built following
[these](/docs/user/sample/viewer) instructions.
