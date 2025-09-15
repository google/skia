/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"

/**
 * This test exercises skbug.com/40032817. An anti-aliased blurred path is rendered through a soft
 * clip. On the GPU a scratch texture was used to hold the original path mask as well as the blurred
 * path result. The same texture is then incorrectly used to generate the soft clip mask for the
 * draw. Thus the same texture is used for both the blur mask and soft mask in a single draw.
 *
 * The correct image should look like a thin stroked round rect.
 */
DEF_SIMPLE_GM_BG(skbug1719, canvas, 300, 100, 0xFF303030) {
        canvas->translate(SkIntToScalar(-800), SkIntToScalar(-650));

        // The data is lifted from an SKP that exhibited the bug.

        // This is a round rect.
        SkPath clipPath = SkPathBuilder()
                          .moveTo(832.f, 654.f)
                          .lineTo(1034.f, 654.f)
                          .cubicTo(1038.4183f, 654.f, 1042.f, 657.58173f, 1042.f, 662.f)
                          .lineTo(1042.f, 724.f)
                          .cubicTo(1042.f, 728.41827f, 1038.4183f, 732.f, 1034.f, 732.f)
                          .lineTo(832.f, 732.f)
                          .cubicTo(827.58173f, 732.f, 824.f, 728.41827f, 824.f, 724.f)
                          .lineTo(824.f, 662.f)
                          .cubicTo(824.f, 657.58173f, 827.58173f, 654.f, 832.f, 654.f)
                          .close()
                          .detach();

        // This is a round rect nested inside a rect.
        SkPath drawPath = SkPathBuilder(SkPathFillType::kEvenOdd)
                          .moveTo(823.f, 653.f)
                          .lineTo(1043.f, 653.f)
                          .lineTo(1043.f, 733.f)
                          .lineTo(823.f, 733.f)
                          .lineTo(823.f, 653.f)
                          .close()
                          .moveTo(832.f, 654.f)
                          .lineTo(1034.f, 654.f)
                          .cubicTo(1038.4183f, 654.f, 1042.f, 657.58173f, 1042.f, 662.f)
                          .lineTo(1042.f, 724.f)
                          .cubicTo(1042.f, 728.41827f, 1038.4183f, 732.f, 1034.f, 732.f)
                          .lineTo(832.f, 732.f)
                          .cubicTo(827.58173f, 732.f, 824.f, 728.41827f, 824.f, 724.f)
                          .lineTo(824.f, 662.f)
                          .cubicTo(824.f, 657.58173f, 827.58173f, 654.f, 832.f, 654.f)
                          .close()
                          .detach();

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFF000000);
        paint.setMaskFilter(
            SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 0.78867501f));
        paint.setColorFilter(SkColorFilters::Blend(0xBFFFFFFF, SkBlendMode::kSrcIn));

        canvas->clipPath(clipPath, true);
        canvas->drawPath(drawPath, paint);
}
