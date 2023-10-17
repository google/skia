`SkTypeface::MakeFromName`, `SkTypeface::MakeFromFile`, `SkTypeface::MakeFromStream`, and
`SkTypeface::MakeFromData` are deprecated and will be removed eventually. These should be replaced
with calls directly to the SkFontMgr that can provide the appropriate typefaces.

`SkTypeface::MakeDefault()` has been deprecated. Soon it will return an empty typeface and
eventually be removed.