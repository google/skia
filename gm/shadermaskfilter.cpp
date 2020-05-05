/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCoverageMode.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkBlendModePriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

static sk_sp<SkShader> make_shader(const SkRect& r) {
    const SkPoint pts[] = {
        { r.fLeft, r.fTop }, { r.fRight, r.fBottom },
    };
    const SkColor colors[] = { 0, SK_ColorWHITE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kRepeat);
}

DEF_SIMPLE_GM(shadermaskfilter_gradient, canvas, 512, 512) {
    SkRect r = { 0, 0, 100, 150 };
    auto shader = make_shader(r);
    auto mf = SkShaderMaskFilter::Make(shader);

    canvas->translate(20, 20);
    canvas->scale(2, 2);

    SkPaint paint;
    paint.setMaskFilter(mf);
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    canvas->drawOval(r, paint);
}
