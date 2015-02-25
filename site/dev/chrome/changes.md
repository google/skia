Chrome changes
==============

If your change modifies the Skia API, you may also need to land a change in Chromium.

The strategy you use to synchronize changes in the Skia and Chromium
repositories may differ based on the nature of the change, but in general, we
recommend using build flag suppressions \(defines\)\.
We also prefer making the old code path opt-in where possible.

Method 1 \(preferred\) \- Make the old code path opt\-in for Chromium

  * Add new code to Skia, leaving the old code in place.
  * Deprecate the old code path so that it must be enabled with a flag such as
    'SK_SUPPORT_LEGACY_XXX'.
  * Synchronize the above changes in Skia with a Chromium commit to
    'skia/skia_common.gypi' or 'skia/config/SkUserConfig.h' to enable the
    deprecated Skia API.
      * Note that the code suppression cannot exist in both the header file and
      the gyp file, it should only reside in one location.
  * Test the new or updated Skia API within Chromium.
  * Remove the flag and code when the legacy code path is no longer in use.

Method 2 \- Make the new code path opt\-in for Chromium

  * Add new code to Skia, suppressed by a flag.
  * Leave the old code path in place.
  * Set the flag in Chromium's 'skia/skia_common.gypi' or
    'skia/config/SkUserConfig.h' to enable the new or updated Skia API.
  * Test the new or updated Skia API within Chromium.
  * Remove the code suppression \(and code\) when the legacy API is no longer
    in use.

If your changes will affect Blink layout tests, see detailed instructions about
how to synchronize the changes between Skia, Blink, and Chromium [here](./blink).
