Fonts and GM Tests
==================

Overview
--------

Each test in the gm directory draws a reference image. Their primary purpose is
to detect when images change unexpectedly, indicating that a rendering bug has
been introduced.

The gm tests have a secondary purpose: they detect when rendering is different
across platforms and configurations.

The dm \(Diamond Master\) tool supports flags that minimize or eliminate the
differences introduced by the font scaler native to each platform.


Portable fonts
--------------

The most portable font format uses Skia to draw characters directly from paths,
and contains a idealized set of font metrics. This does not exercise platform
specific fonts at all, but does support specifying the font name, font size,
font style, and attributes like fakeBold. The paths are generated on a reference
platform \(currently a Mac\) and are stored as data in
'tools/test_font_data.cpp' .

To use portable fonts, pass '\-\-portableFonts' to dm.


Resource fonts
--------------

The '\-\-resourceFonts' flag directs dm to use font files present in the resources
directory. By using the same font set on all buildbots, the generated gm images
become more uniform across platforms.

Today, the set of fonts used by gm, and present in my resources directory,
include:

  * Courier New Bold Italic.ttf
  * Courier New Bold.ttf
  * Courier New Italic.ttf
  * Courier New.ttf
  * LiberationSans-Bold.ttf
  * LiberationSans-BoldItalic.ttf
  * LiberationSans-Italic.ttf
  * LiberationSans-Regular.ttf
  * Papyrus.ttc
  * Pro W4.otf
  * Times New Roman Bold Italic.ttf
  * Times New Roman Bold.ttf
  * Times New Roman Italic.ttf
  * Times New Roman.ttf 


System fonts
------------

If neither '\-\-portableFonts' nor '\-\-resourceFonts' is specified, dm uses the fonts
present on the system. Also, if '\-\-portableFonts' or '\-\-resourceFonts' is specified
and the desired font is not available, the native font lookup algorithm is
invoked.


GM font selection
-----------------

Each gm specifies the typeface to use when drawing text. For now, to set the
portable typeface on the paint, call:

~~~~
sk_tool_utils::set_portable_typeface(SkPaint* , const char* name = nullptr,
SkTypeface::Style style = SkTypeface::kNormal );
~~~~

To create a portable typeface, use:

~~~~
SkTypeface* typeface = sk_tool_utils::create_portable_typeface(const char* name,
SkTypeface::Style style);
~~~~

Eventually, both 'set_portable_typeface()' and 'create_portable_typeface()' will be
removed. Instead, a test-wide 'SkFontMgr' will be selected to choose portable
fonts or resource fonts.


Adding new fonts and glyphs to a GM
-----------------------------------

If a font is missing from the portable data or the resource directory, the
system font is used instead. If a glyph is missing from the portable data, the
first character, usually a space, is drawn instead.

Running dm with '\-\-portableFonts' and '\-\-reportUsedChars' generates
'tools/test_font_data_chars.cpp', which describes the fonts and characters used by
all gm tests. Subsequently running the 'create_test_font' tool generates new paths
and writes them into 'tools/test_font_data.cpp' .


Future work
-----------

The font set used by gm tests today is arbitrary and not intended to be
cross-platform. By choosing fonts without licensing issues, all bots can freely
contain the same fonts. By narrowing the font selection, the size of the test
font data will be more manageable.

Adding support for selecting from multiple font managers at runtime permits
removing manual typeface selection in the gm tests. Today, options to dm like
'\-\-pipe' fail with '\-\-portableFonts' because we're hard-coded to using the default
font manage when pictures are serialized.

Some gm tests explicitly always want to use system fonts and system metrics;
other gm tests use text only to label the drawing; yet other gm tests use text
to generate paths for testing. Additional discrimination is needed to
distinguish these cases.
