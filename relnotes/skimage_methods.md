`SkImage::makeColorSpace` and `SkImage::makeColorTypeAndColorSpace` now take a `GrDirectContext`
as the first parameter. This should be supplied when dealing with texture-backed images and can
be `nullptr` otherwise.