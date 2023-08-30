
`SkImage::makeWithFilter` has been deprecated. It has been replaced with three factory functions:

Ganesh:   `SkImages::MakeWithFilter(GrRecordingContext*, ...);`         -- declared in SkImageGanesh.h

Graphite: `SkImages::MakeWithFilter(skgpu::graphite::Recorder*, ...);`  -- declared in Image.h

Raster:   `SkImages::MakeWithFilter(...);`                              -- declared in SkImage.h

The new factories require the associated backend context object be valid. For example, the Graphite version will return nullptr if it isn't supplied with a `Recorder` object.
