
---
title: "Fonts and GM Tests"
linkTitle: "Fonts and GM Tests"

---


Overview
--------

Each test in the gm directory draws a reference image. Their primary purpose is
to detect when images change unexpectedly, indicating that a rendering bug has
been introduced.

The gm tests have a secondary purpose: they detect when rendering is different
across platforms and configurations.

GM font selection
-----------------

Each gm specifies the typeface to use when drawing text. To create a portable
typeface, use:

~~~~
SkTypeface* typeface = ToolUtils::CreatePortableTypeface(const char* name,
SkFontStyle style);
~~~~

