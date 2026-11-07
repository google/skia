`SkTextBlob::getIntercepts()`, `SkOverdrawColorFilter::MakeWithSkColors()`,
`SkStrikeRef::getWidthsStrided()`, and `SkRuntimeEffect::makeColorFilter()` (children + count
form) now have overloads that take `SkSpan` instead of raw pointers (for `getIntercepts()`, the
`bounds` argument), so buffer sizes are checked. The existing pointer-based overloads are
deprecated; they remain available and forward to the new `SkSpan` overloads, but clients should
migrate to the `SkSpan` versions. Note that C-style array arguments still select the
pointer-based overloads.
