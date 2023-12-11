`SkFont::getTypeface()` will no longer return a nullptr to indicate "the default typeface".
If left unspecified, SkFonts will use an empty typeface (e.g. no glyphs).