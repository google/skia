`SkGraphics::PurgePinnedFontCache()` has been added to allow clients to
explicitly trigger `SkStrikeCache` purge checks for `SkStrikes` with
pinners. Defining `SK_STRIKE_CACHE_DOESNT_AUTO_CHECK_PINNERS` in the
user configuration now disables automatic purge checking of strikes with
pinners.
