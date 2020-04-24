Fonts and GM Tests
==================

Overview
--------

Each test in the gm directory draws a reference image. Their primary purpose is
to detect when images change unexpectedly, indicating that a rendering bug has
been introduced.

The gm tests have a secondary purpose: they detect when rendering is different
across platforms and configurations.

GM font selection
-----------------

Each gm specifies the typeface to use when drawing text. For now, to set the
portable typeface on the paint, call:

~~~~
ToolUtils::set_portable_typeface(SkPaint* , const char* name = nullptr,
SkFontStyle style = SkFontStyle());
~~~~

To create a portable typeface, use:

~~~~
SkTypeface* typeface = ToolUtils::create_portable_typeface(const char* name,
SkFontStyle style);
~~~~

Eventually, both `set_portable_typeface()` and `create_portable_typeface()` will be
removed. Instead, a test-wide `SkFontMgr` will be selected to choose portable
fonts or resource fonts.
