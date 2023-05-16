`SkImageFilters::Picture` now returns a non-null image filter if the input `sk_sp<SkPicture>` is
null. The returned filter evaluates to transparent black, which is equivalent to a null or empty
picture. Previously, returning a null image filter would mean that the dynamic source image could
be surprisingly injected into the filter evaluation where it might not have been intended.
