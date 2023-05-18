`SkImageFilters::Image` now returns a non-null image filter if the input `sk_sp<SkImage>` is
null or the src rectangle is empty or does not overlap the image. The returned filter evaluates to
transparent black, which is equivalent to a null or empty image. Previously, returning a null image
filter would mean that the dynamic source image could be surprisingly injected into the filter
evaluation where it might not have been intended.
