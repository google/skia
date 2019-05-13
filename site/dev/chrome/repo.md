Working in a Chromium repo
==========================

To work on Skia inside a Chromium checkout, run the following:

    cd chromium/src/third_party/skia
    python2 tools/git-sync-deps
    bin/gn gen out/Debug

The second command does a minimal "just sync the DEPS" emulation of `gclient
sync` for Skia into chromium/src/third_party/skia/third_party.  After that,
`ninja -C out/Debug dm` in chromium/src/third_party/skia will get you rolling.

We no longer recommend the .gclient file manipulation to have Chromium DEPS also
sync Skia's DEPS.  Most of those DEPS are for building and testing only;
Chromium doesn't need any of them, and it can be confusing and problematic if
they somehow get mixed into the Chromium build.
