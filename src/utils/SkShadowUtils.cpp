/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkShadowUtils.h"
#include "SkCanvas.h"
#include "../effects/shadows/SkAmbientShadowMaskFilter.h"
#include "../effects/shadows/SkSpotShadowMaskFilter.h"

// Draw an offset spot shadow and outlining ambient shadow for the given path.
void SkShadowUtils::DrawShadow(SkCanvas* canvas, const SkPath& path, SkScalar occluderHeight,
                               const SkPoint3& lightPos, SkScalar lightRadius,
                               SkScalar ambientAlpha, SkScalar spotAlpha, SkColor color,
                               uint32_t flags) {
    SkPaint newPaint;
    newPaint.setColor(color);
    newPaint.setMaskFilter(SkAmbientShadowMaskFilter::Make(occluderHeight, ambientAlpha, flags));
    canvas->drawPath(path, newPaint);
    newPaint.setMaskFilter(SkSpotShadowMaskFilter::Make(occluderHeight, lightPos, lightRadius,
                                                        spotAlpha, flags));
    canvas->drawPath(path, newPaint);
}
