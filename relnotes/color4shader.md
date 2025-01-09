`SkShaders::Color(SkColor4f, sk_sp<SkColorSpace>)` now always applies the color
space to the color, even if rendering to a legacy `SkSurface` that is not
color managed. In this case, the target color space is assumed to be sRGB.
