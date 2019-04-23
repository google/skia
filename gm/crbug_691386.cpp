/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/utils/SkParsePath.h"

DEF_SIMPLE_GM_CAN_FAIL(crbug_691386, canvas, errorMsg, 256, 256) {
    SkPath path;
    if (!SkParsePath::FromSVGString("M -1 0 A 1 1 0 0 0 1 0 Z", &path)) {
        *errorMsg = "Failed to parse path.";
        return skiagm::DrawResult::kFail;
    }
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(0.025f);
    canvas->scale(96.0f, 96.0f);
    canvas->translate(1.25f, 1.25f);
    canvas->drawPath(path, p);
    return skiagm::DrawResult::kOk;
}
