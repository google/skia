Added `SkStrikeRef`, a public lightweight handle to a resolved font strike.
`SkFont::makeStrikeRef()` returns an `SkStrikeRef` that allows repeated glyph
metric queries (advances, bounds) without the per-call overhead of strike
lookup. This is useful for text shaping engines that make many `getWidths`
calls with the same font configuration.
