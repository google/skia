
---
title: "Blink layout tests"
linkTitle: "Blink layout tests"

---


How to land Skia changes that change Blink layout test results.

Changes that affect a small number of layout test results
---------------------------------------------------------
Changes affecting fewer than ~20 layout tests can be rebaselined without
special coordination with the Blink gardener using these steps:

1. Prepare your Skia change, taking note of which layout tests will turn red
   \(see http://www.chromium.org/developers/testing/webkit-layout-tests for more
   detail on running the Blink layout tests\).
2. Check in your code to the Skia repo.
3. Ahead of the Skia auto roll including your change, manually push a change to the
   Blink LayoutTests/TestExpectations [file](https://chromium.googlesource.com/chromium/src/+/main/third_party/blink/web_tests/TestExpectations), flagging tests expected to fail as a result of your change as follows:
   foo/bar/test-name.html [ Failure Pass ]  # Needs rebaseline

4. Wait for the Skia roll to land successfully.
5. Check in another change to the Blink TestExpectations file removing all the
  skipped test expectations you add earlier, an run `git cl rebaseline` which will prompt the automatic rebaseline.



Changes that affect a large number of test results
--------------------------------------------------
Where a 'large number' or 'many' means more than about 20.
Follow the instructions below:

In the following the term 'code suppression' means a build flag \(a\.k\.a\. define\).
Such code suppressions should be given a name with the form SK\_IGNORE\_xxx\_FIX.

Updating the version of Skia in Chromium is called a 'roll'.
The Auto Roll Bot performs this roll multiple times per day, and can also be done manually.
See https://chromium.googlesource.com/chromium/src/+log/main/DEPS and search for skia\-deps\-roller.

### Setup
#### Code suppression does not yet exist \- Direct method
1. Make a change in Skia which will change many Blink layout tests.
2. Put the change behind a code suppression.
3. Check in the change to the Skia repository.
4. Manually roll Skia or append the autoroll with the code suppression to
   Chromium's 'skia/chromium\_skia\_defines\.gypi'

#### Code suppression does not yet exist \- Alternate method
1. Add code suppression to Chromium's 'skia/chromium\_skia\_defines\.gypi' before making code
   changes in Skia.
2. Make a change in Skia which will change many Blink layout tests.
3. Put the change behind a code suppression.
4. Check in the change to the Skia repository.
5. Wait for Skia roll into Chromium.

#### Code suppression exists in header
1. Remove code suppression from header file in Chromium and add code suppression to
   Chromium's 'skia/chromium\_skia\_defines\.gypi'.
   The code suppression cannot be in a header file and a defined in a gyp file at the
   same time or a multiple definition warning will be treated as an error and break
   the Chromium build.

### Rebaseline
1. Choose a time when the Blink tree is likely to be quiet. Avoid PST afternoons in
   particular. The bigger the change, the more important this is. Regardless,
   determine who the Blink gardener is and notify them. You will be making the
   Chromium\.WebKit tree very red for an extended period, and the gardener needs to
   know that they are not expected to fix it.
2. Create a CL removing the code suppression from Chromium's
   skia/chromium\_skia\_defines\.gypi while simultaneously adding [ NeedsRebaseline ]
   lines to Blink's LayoutTests/TestExpectations [file](https://chromium.googlesource.com/chromium/src/+/main/third_party/blink/web_tests/TestExpectations).
   Then the auto rebaseline bot will take care of the work of actually checking in the
   new images. This is generally acceptable for up to 600 or so rebaselined images.
   Above that you might still use [ NeedsRebaseline ], but it's best to coordinate with
   the gardener. This should go through the CQ cleanly.
3. Be careful with tests that are already failing or flakey. These may or may not need
   to be rebaselined and flakey tests should not be removed from TestExpectations
   regardless. In such cases revert the TestExpectations changes before committing.
4. If you are not the one handling the cleanup step, please open a Skia Issue of the
   form
   Title: "Remove code suppression SK\_IGNORE\_xxx\_FIX\."
   Comment: "Code suppression SK\_IGNORE\_xxx\_FIX rebaselined with Blink revision
   123456\." and assign it to the individual responsible for the cleanup step.

### Cleanup
1. Remove the now unused old code from Skia and any defines which were introduced
   to suppress the new code.
2. Check in the cleanup change to the Skia repository.
3. Wait for Skia roll into Chromium.


