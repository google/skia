/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurImageFilter_DEFINED
#define SkBlurImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

enum class SkTileMode;

class SK_API SkBlurImageFilter {
public:
    /*! \enum TileMode
     * DEPRECATED: Use SkTileMode instead. */
    enum TileMode {
      kClamp_TileMode = 0,    /*!< Clamp to the image's edge pixels. */
                              /*!< This re-weights the filter so samples outside have no effect */
      kRepeat_TileMode,       /*!< Wrap around to the image's opposite edge. */
      kClampToBlack_TileMode, /*!< Fill with transparent black. */
      kLast_TileMode = kClampToBlack_TileMode,

      // TODO: remove kMax - it is non-standard but Chromium uses it
      kMax_TileMode = kClampToBlack_TileMode
    };

    static sk_sp<SkImageFilter> Make(SkScalar sigmaX, SkScalar sigmaY,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr,
                                     TileMode tileMode = TileMode::kClampToBlack_TileMode);
    // EXPERIMENTAL: kMirror is not yet supported
    static sk_sp<SkImageFilter> Make(SkScalar sigmaX, SkScalar sigmaY, SkTileMode tileMode,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilter::CropRect* cropRect = nullptr);

    static void RegisterFlattenables();

private:
    SkBlurImageFilter() = delete;
};

#endif
