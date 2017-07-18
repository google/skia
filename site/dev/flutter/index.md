Skia in Flutter & Fuchsia
=========================

Skia is used by both [Flutter](https://flutter.io/) and [Fuchsia](https://fuchsia.googlesource.com/docs/+/master/README.md).

Fuchsia has a [roller](https://fuchsia-roll.skia.org/) that will continuously roll latest Skia into that project. Fuchsia uses an XML [manifest](https://fuchsia.googlesource.com/manifest/+/master/userspace) to specify the Skia revision (as well as other third party libraries).

Flutter does not (yet) have a roller, so developers must manually perform rolls. Flutter uses [DEPS](https://github.com/flutter/engine/blob/master/DEPS) to specify third party dependencies.


Although each project is (almost always) building at a different revision of Skia, Fuchsia itself always builds the latest revision of Flutter as one of its components. Thus, the versions of Skia being used by Flutter and Fuchsia must be "source compatible" -- Flutter must be capable of compiling against either revision without any change to Flutter itself.

Making API Changes
------------------

If you need to make a breaking API change, the basic approach is:

* Add new code to Skia, leaving the old code in place.
* Deprecate the old code path so that it must be enabled with a flag such as 'SK_SUPPORT_LEGACY_XXX'.
* Add that same flag to [flutter\_defines.gni](https://skia.googlesource.com/skia/+/master/gn/flutter_defines.gni) in Skia.
  * Both Flutter and Fuchsia build Skia with a GN argument that enables all the defines listed in that file.
* Land the Skia change, and test the new API in both Flutter and Fuchsia.
* Remove the flag and code when the legacy code path is no longer in use.
