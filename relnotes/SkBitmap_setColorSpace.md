Added `SkBitmap::setColorSpace`. This API allows the colorspace of an existing
`SkBitmap` to be reinterpreted. The pixel data backing the bitmap will be left
as-is. The colorspace will be honored when the bitmap is accessed via APIs which
support colorspace conversion, like `readPixels`.
