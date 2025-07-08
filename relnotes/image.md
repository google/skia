`SkImage::isValid(GrRecordingContext*)` has been deprecated in favor of the `SkRecorder*` version.
To migrate do something like `image->isValid(ctx->asRecorder())`.

`SkImage::makeSubset(GrDirectContext*, ...)` has been deprecated in favor of the `SkRecorder*`
version. To migrate, do something like `image->makeSubset(ctx->asRecorder, ..., {})`

`SkImage::makeColorSpace(GrDirectContext*, ...)` has been deprecated in favor of the `SkRecorder*`
version. To migrate, do something like `image->makeColorSpace(ctx->asRecorder, ..., {})`

`SkImage::makeColorTypeAndColorSpace(GrDirectContext*, ...)` has been deprecated in favor of the
`SkRecorder*` version. To migrate, do something like
`image->makeColorTypeAndColorSpace(ctx->asRecorder, ..., {})`

In the case you are working with CPU-backed images, `skcpu::Recorder::TODO()` should work until
a `skcpu::Context` and `skcpu::Recorder` can be used properly.