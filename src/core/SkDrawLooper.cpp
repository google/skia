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

void SkDrawLooper::Context::Info::applyToCTM(SkMatrix* ctm) const {
    if (fApplyPostCTM) {
        ctm->postTranslate(fTranslate.fX, fTranslate.fY);
    } else {
        ctm->preTranslate(fTranslate.fX, fTranslate.fY);
    }
}

void SkDrawLooper::Context::Info::applyToCanvas(SkCanvas* canvas) const {
    if (fApplyPostCTM) {
        SkMatrix ctm = canvas->getTotalMatrix();
        ctm.postTranslate(fTranslate.fX, fTranslate.fY);
        canvas->setMatrix(ctm);
    } else {
        canvas->translate(fTranslate.fX, fTranslate.fY);
    }
}

bool SkDrawLooper::canComputeFastBounds(const SkPaint& paint) const {
    SkSTArenaAlloc<48> alloc;

    SkDrawLooper::Context* context = this->makeContext(&alloc);
    for (;;) {
        SkPaint p(paint);
        SkDrawLooper::Context::Info info;
        if (context->next(&info, &p)) {
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

    SkSTArenaAlloc<48> alloc;

    *dst = src;   // catch case where there are no loops
    SkDrawLooper::Context* context = this->makeContext(&alloc);

    for (bool firstTime = true;; firstTime = false) {
        SkPaint p(paint);
        SkDrawLooper::Context::Info info;
        if (context->next(&info, &p)) {
            SkRect r(src);

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
            p.setLooper(nullptr);
#endif
            p.computeFastBounds(r, &r);
            r.offset(info.fTranslate.fX, info.fTranslate.fY);

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
    Context* ctx = this->makeContext(&alloc);
    if (ctx) {
        Context::Info info;
        for (;;) {
            SkPaint p = paint;
            if (!ctx->next(&info, &p)) {
                break;
            }
            canvas->save();
            if (info.fApplyPostCTM) {
                SkMatrix ctm = canvas->getTotalMatrix();
                ctm.postTranslate(info.fTranslate.fX, info.fTranslate.fY);
                canvas->setMatrix(ctm);
            } else {
                canvas->translate(info.fTranslate.fX, info.fTranslate.fY);
            }
            proc(canvas, p);
            canvas->restore();
        }
    }
}
