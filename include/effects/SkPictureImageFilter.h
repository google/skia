/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureImageFilter_DEFINED
#define SkPictureImageFilter_DEFINED

#include "SkDrawableImageFilter.h"

// Utility for creating SkpictureImageFilters from SkPictures. See
// SkpictureImageFilter for more comments.
class SK_API SkPictureImageFilter : public SkDrawableImageFilter {
public:
  static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture);

  static sk_sp<SkImageFilter> Make(sk_sp<SkPicture> picture, const SkRect& cropRect);

  static sk_sp<SkImageFilter> MakeForLocalSpace(sk_sp<SkPicture> picture,
                                                const SkRect& cropRect,
                                                SkFilterQuality filterQuality);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPictureImageFilter)

private:
  SkPictureImageFilter(sk_sp<SkPicture> picture);
  SkPictureImageFilter(sk_sp<SkPicture> picture, const SkRect& cropRect,
                       DrawableResolution, SkFilterQuality, sk_sp<SkColorSpace>);

  typedef SkDrawableImageFilter INHERITED;
};

#endif
