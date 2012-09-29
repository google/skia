
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMaskFilter.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkDraw.h"
#include "SkRasterClip.h"

SK_DEFINE_INST_COUNT(SkMaskFilter)

bool SkMaskFilter::filterMask(SkMask*, const SkMask&, const SkMatrix&,
                              SkIPoint*) {
    return false;
}

bool SkMaskFilter::filterPath(const SkPath& devPath, const SkMatrix& matrix,
                              const SkRasterClip& clip, SkBounder* bounder,
                              SkBlitter* blitter, SkPaint::Style style) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), this, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode,
                            style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.fImage);

    if (!this->filterMask(&dstM, srcM, matrix, NULL)) {
        return false;
    }
    SkAutoMaskFreeImage autoDst(dstM.fImage);

    // if we get here, we need to (possibly) resolve the clip and blitter
    SkAAClipBlitterWrapper wrapper(clip, blitter);
    blitter = wrapper.getBlitter();

    SkRegion::Cliperator clipper(wrapper.getRgn(), dstM.fBounds);

    if (!clipper.done() && (bounder == NULL || bounder->doIRect(dstM.fBounds))) {
        const SkIRect& cr = clipper.rect();
        do {
            blitter->blitMask(dstM, cr);
            clipper.next();
        } while (!clipper.done());
    }

    return true;
}

SkMaskFilter::BlurType SkMaskFilter::asABlur(BlurInfo*) const {
    return kNone_BlurType;
}

void SkMaskFilter::computeFastBounds(const SkRect& src, SkRect* dst) {
    SkMask  srcM, dstM;

    srcM.fImage = NULL;
    src.roundOut(&srcM.fBounds);
    srcM.fRowBytes = 0;
    srcM.fFormat = SkMask::kA8_Format;

    SkIPoint margin;    // ignored
    if (this->filterMask(&dstM, srcM, SkMatrix::I(), &margin)) {
        dst->set(dstM.fBounds);
    } else {
        dst->set(srcM.fBounds);
    }
}


