`SkCodec::getImage()` will now respect the origin in the metadata (e.g. Exif metadata that
rotates the image). This may mean callers who provide an SkImageInfo may need to rotate it,
e.g. via `SkPixmapUtils::SwapWidthHeight`.