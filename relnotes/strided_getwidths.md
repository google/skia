Add a getWidthsStrided() API to SkStrikeRef which allows scattered memory access
into input glyph list and provides scattered writes with a stride length into a
client provided output buffer.  Useful for fast access and transfer of advance
widths in shaping. Shown to improve Blink performance.
