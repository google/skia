SkPoint now uses float for its coordinates. This starts the process of removing SkScalar from Skia.
SkScalar was a typedef for float, so this has no practical impact on code that uses Skia.
