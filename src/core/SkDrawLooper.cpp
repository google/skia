/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDrawLooper.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRect.h"

SK_DEFINE_INST_COUNT(SkDrawLooper)

bool SkDrawLooper::canComputeFastBounds(const SkPaint& paint) {
    SkCanvas canvas;

    this->init(&canvas);
    for (;;) {
        SkPaint p(paint);
        if (this->next(&canvas, &p)) {
            p.setLooper(NULL);
            if (!p.canComputeFastBounds()) {
                return false;
            }
        } else {
            break;
        }
    }
    return true;
}

void SkDrawLooper::computeFastBounds(const SkPaint& paint, const SkRect& src,
                                     SkRect* dst) {
    SkCanvas canvas;

    *dst = src;   // catch case where there are no loops
    this->init(&canvas);
    for (bool firstTime = true;; firstTime = false) {
        SkPaint p(paint);
        if (this->next(&canvas, &p)) {
            SkRect r(src);

            p.setLooper(NULL);
            p.computeFastBounds(r, &r);
            canvas.getTotalMatrix().mapRect(&r);

            if (firstTime) {
                *dst = r;
            } else {
                dst->join(r);
            }
        } else {
            break;
        }
    }
}
