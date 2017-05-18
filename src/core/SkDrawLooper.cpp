/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkDrawLooper.h"
#include "SkCanvas.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRect.h"

bool SkDrawLooper::canComputeFastBounds(const SkPaint& paint) const {
    SkCanvas canvas;
    char storage[48];
    SkArenaAlloc alloc {storage};

    SkDrawLooper::Context* context = this->makeContext(&canvas, &alloc);
    for (;;) {
        SkPaint p(paint);
        if (context->next(&canvas, &p)) {
            p.setLooper(nullptr);
            if (!p.canComputeFastBounds()) {
                return false;
            }
        } else {
            break;
        }
    }
    return true;
}

void SkDrawLooper::computeFastBounds(const SkPaint& paint, const SkRect& s,
                                     SkRect* dst) const {
    // src and dst rects may alias and we need to keep the original src, so copy it.
    const SkRect src = s;

    SkCanvas canvas;
    char storage[48];
    SkArenaAlloc alloc {storage};

    *dst = src;   // catch case where there are no loops
    SkDrawLooper::Context* context = this->makeContext(&canvas, &alloc);

    for (bool firstTime = true;; firstTime = false) {
        SkPaint p(paint);
        if (context->next(&canvas, &p)) {
            SkRect r(src);

            p.setLooper(nullptr);
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

bool SkDrawLooper::asABlurShadow(BlurShadowRec*) const {
    return false;
}
