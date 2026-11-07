/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

class SkColorFilter;

#ifndef SkOverdrawColorFilter_DEFINED
#define SkOverdrawColorFilter_DEFINED

/**
 *  Uses the value in the src alpha channel to set the dst pixel.
 *  0             -> colors[0]
 *  1             -> colors[1]
 *  ...
 *  5 (or larger) -> colors[5]
 *
 *  https://fiddle.skia.org/c/@overdrawcolorfilter_grid
 */
class SK_API SkOverdrawColorFilter {
public:
    static constexpr int kNumColors = 6;

    static sk_sp<SkColorFilter> MakeWithSkColors(SkSpan<const SkColor, kNumColors>);

    /** DEPRECATED: Prefer the SkSpan overload above, which enforces the color count.
        This pointer overload is kept for existing clients and forwards to it.

        PRECONDITIONS: colors must point to at least kNumColors readable elements.
    */
    static sk_sp<SkColorFilter> MakeWithSkColors(const SkColor colors[kNumColors]);
};

#endif // SkOverdrawColorFilter_DEFINED
