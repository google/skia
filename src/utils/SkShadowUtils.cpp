/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "../effects/SkShadowMaskFilter.h"

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& lightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {
    SkPaint newPaint;
    newPaint.setColor(color);
    newPaint.setMaskFilter(SkShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
                                                    ambientAlpha, spotAlpha, flags));

    canvas->drawPath(path, newPaint);
}
