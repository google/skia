`SkScalarIsFinite`, `SkScalarsAreFinite`, and `SkScalarIsNaN` have been removed from the Skia API.
These calls can be replaced with the functionally-equivalent `std::isfinite` and `std::isnan`.
