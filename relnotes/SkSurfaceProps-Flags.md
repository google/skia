A `kDefault_Flag = 0` value has been added to the `SkSurfaceProps::Flags` enum. This is just a
self-documenting zero-value that aims to improve code readability, e.g.:

```
// The two lines below are equivalent.

SkSurfaceProps(/* surfaceFlags= */ 0, kRGB_H_SkPixelGeometry);

SkSurfaceProps(SkSurfaceProps::kDefault_Flag, kRGB_H_SkPixelGeometry);
```
