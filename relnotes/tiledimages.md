A new `SkTiledImageUtils` namespace (in `SkTiledImageUtils.h`) provides `DrawImage` and `DrawImageRect` methods that directly mirror `SkCanvas'` `drawImage` and `drawImageRect` calls.

The new entry points will breakup large `SkBitmap`-backed `SkImages` into tiles and draw them if they would be too large to upload to the gpu as one texture.

They will fall through to their `SkCanvas` correlates if tiling isn't needed or possible.
