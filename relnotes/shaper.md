Harfbuzz-backed SkShaper instances will no longer treat a null SkFontMgr as meaning "use the
default SkFontMgr for fallback" and instead will *not* do fallback for glyphs missing from a font.