/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "src/core/SkArenaAlloc.h"

bool SkDrawLooper::canComputeFastBounds(const SkPaint& paint) const {
    SkCanvas canvas;
    SkSTArenaAlloc<48> alloc;

    SkDrawLooper::Context* context = this->makeContext(&canvas, &alloc);
    for (;;) {
        SkPaint p(paint);
        if (context->next(&canvas, &p)) {
#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
            p.setLooper(nullptr);
#endif
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
    SkSTArenaAlloc<48> alloc;

    *dst = src;   // catch case where there are no loops
    SkDrawLooper::Context* context = this->makeContext(&canvas, &alloc);

    for (bool firstTime = true;; firstTime = false) {
        SkPaint p(paint);
        if (context->next(&canvas, &p)) {
            SkRect r(src);

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
            p.setLooper(nullptr);
#endif
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

void SkDrawLooper::apply(SkCanvas* canvas, const SkPaint& paint,
                         std::function<void(SkCanvas*, const SkPaint&)> proc) {
    SkSTArenaAlloc<256> alloc;
    Context* ctx = this->makeContext(canvas, &alloc);
    if (ctx) {
        for (;;) {
            SkPaint p = paint;
            if (!ctx->next(canvas, &p)) {
                break;
            }
            proc(canvas, p);
        }
    }
}
