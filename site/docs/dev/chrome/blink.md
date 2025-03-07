
---
title: "Blink layout tests"
linkTitle: "Blink layout tests"

---

How to land Skia changes that change Blink layout test results.
See https://chromium.googlesource.com/chromium/src/+/HEAD/docs/testing/web_tests.md for more
detail on running the Blink layout tests.

General tips about layout tests
-------------------------------
* Layout tests come in two flavors: "compare 2 html pages" and "compare html page and .png file"
  When rebaselining, most of the effort comes from regenerating the .png files for the second
  kind. The first kind will be something like `third_party/blink/web_tests/.../something.html`
  and have a companion file `.../something-expected.html` as a sibling file.
  ([example](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/web_tests/fast/forms/text/input-appearance-autocomplete-very-long-value.html;drc=f68d6358bed8ebfc88a0198d6cda50256620c71d);
  [companion html](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/web_tests/fast/forms/text/input-appearance-autocomplete-very-long-value-expected.html;drc=f68d6358bed8ebfc88a0198d6cda50256620c71d))
  The second type won't have the companion html file, but might have a companion .png file, or
  multiple .png files in other directories when the html should render differently
  on other platforms or settings.
  ([example](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/web_tests/dark-mode/images/opt-out-svg-gradient.html;drc=44ad10338113aab1779d81df359aca34da89daf3);
  [expected png](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/web_tests/virtual/dark-mode-default/dark-mode/images/opt-out-svg-gradient-expected.png;l=1;drc=ec59d7b96e81ccc0e3dc497697e23304d7259b09))
  For this second type, using <https://cs.chromium.org> is a good way to find these if you need to
  look at rebaselining history or something

* Layout tests (of both kinds) can be given fuzzy matching by adding a meta HTML tag to the test
  file.
  `<meta name="fuzzy" content="maxDifference=0-4; totalPixels=0-100" />`

* Some non-layout tests (also called pixel tests) will fail as a result of rendering changes because
  they have their own checked-in images. Look at the logs of failing tests, as these will hopefully
  output base64 encoded pngs of the expected and actual image. Open up a browser tab, use Dev Tools
  to create an `<img src="[base64]" />` with the actual base64 data and right-click to save the
  image as the new expected data.
  ([example](https://source.chromium.org/chromium/chromium/src/+/main:chrome/browser/ui/views/accessibility/accessibility_focus_highlight_browsertest.cc;l=238;drc=a48632411d7e7263e8fd4d273d24a80f668b73ec);
   [expected png](https://source.chromium.org/chromium/chromium/src/+/main:chrome/test/data/accessibility/focus_highlight_appearance.png;l=1;drc=1e2dbf6a77e2f7264da0097a3cd4158c249a75b8))

* Some tests compare [Skia and PyCairo](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/web_tests/external/wpt/html/canvas/tools/README.md).
  Since Skia makes different choices to Cairo, it's best to increase fuzzy tolerance for these. Look
  for a `fuzzy` entry in the .yaml file that generates the tests and then regenerate things (or just
  use find and replace 🫣).

* Failing CQ tests usually have a "Show Reproduction Instructions" for running locally. This can help verify
  fuzzy tolerances. Be sure to use [--gtest_filter](https://github.com/google/googletest/blob/main/docs/advanced.md#running-a-subset-of-the-tests)
  to limit what you are testing for faster iteration time.

Changes that affect a large number of test results
--------------------------------------------------
Where a 'large number' or 'many' means more than about 20, it's a bit of a process to get
things rebaselined in Chromium.

1. Add a staging define to the Skia code that allows a client to (at compile time) opt-in to
   the old code path. If only Chromium needs rebaselining, it's probably easier to set it up
   like `if !defined(SK_USE_LEGACY_xxx)`. If this needs to be staged across multiple clients,
   `if defined(SK_USE_NEW_xxx)` is better to let clients "opt-in" one at a time.
   ([example CL](https://crrev.com/c/6316987))
2. Tell Chromium to use the old code path using a staging define in their `SkUserConfig.h`
   (or their `//skia/BUILD.gn` if it impacts only specific builds).
   ([example CL](https://crrev.com/c/6316987))
3. Land and wait for the autoroller to roll Skia into Chromium.
   ([example CL](http://review.skia.org/953516) [autoroll CL](https://crrev.com/c/6324680))
4. Create Chromium CL to use the new code path (by removing the define) and update expectations.
   Follow [the rebaselining steps](https://chromium.googlesource.com/chromium/src/+/HEAD/docs/testing/web_test_expectations.md#How-to-rebaseline) to update layout tests that use a reference image.
   For other types of tests (including the .html and -expected.html type), observe the above tips
   to manually update them. To update the images, you may have to repeat the flow of "sync",
   "run tryjobs" and "rebaseline images from them" a few times due to other simultaneous changes or
   flaky tests. Feel free to add some `<meta name="fuzzy"` tags to any of the flaky tests.

   ([example CL](https://crrev.com/c/6328778))
5. Remove staging define from Skia.
   ([example CL](http://review.skia.org/960516))

Changes that affect a small number of layout test results
---------------------------------------------------------
Changes affecting fewer than ~20 layout tests can be rebaselined without a staging define using
these steps:

1. Prepare your Skia change. Run the `Chromium-Canary` tryjob and take note of which layout tests
   will turn red.
2. Ahead of the Skia auto roll including your change, manually push a change to the
   Blink LayoutTests/TestExpectations [file](https://chromium.googlesource.com/chromium/src/+/main/third_party/blink/web_tests/TestExpectations), flagging tests expected to fail as a result of your change as
   follows:
   ```
   foo/bar/test-name.html [ Failure Pass ]  # Needs rebaseline
   ```
   There's a section `Skia roll test suppressions` to use (to avoid conflicts with other changes).
3. Check in your code to the Skia repo.
4. Wait for the Skia roll to land successfully.
5. In your Chromium checkout, create a new branch
  (e.g. `git co main && gclient sync -D && git cl new-branch update-expectations`).
  Follow [the rebaselining steps](https://chromium.googlesource.com/chromium/src/+/HEAD/docs/testing/web_test_expectations.md#How-to-rebaseline)
  and remove the suppressions from `TestExpectations`.
