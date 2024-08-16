`SkScalerContext::MakeRecAndEffects` now converts `SkFont::isEmbolden` to the `kEmbolden_Flag`.
It no longer automatically converts embolden requests into (more) stroking.
This can now (optionally) be done in `SkTypeface::onFilterRec` by calling the new `SkScalerContextRec::useStrokeForFakeBold()`.

