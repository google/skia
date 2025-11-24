`SkImage::refEncodedData()` and `SkImageGenerator::refEncodedData()` now returns a pointer to
const SkData to more explicitly signal that this is a read-only view into the data.

