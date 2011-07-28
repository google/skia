
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMaskFilter.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkBuffer.h"
#include "SkDraw.h"
#include "SkRegion.h"

bool SkMaskFilter::filterMask(SkMask*, const SkMask&, const SkMatrix&,
                              SkIPoint*) {
    return false;
}

bool SkMaskFilter::filterPath(const SkPath& devPath, const SkMatrix& matrix,
                              const SkRegion& clip, SkBounder* bounder,
                              SkBlitter* blitter) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), this, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
        return false;
    }

    SkAutoMaskImage autoSrc(&srcM, false);

    if (!this->filterMask(&dstM, srcM, matrix, NULL)) {
        return false;
    }

    SkAutoMaskImage         autoDst(&dstM, false);
    SkRegion::Cliperator    clipper(clip, dstM.fBounds);

    if (!clipper.done() && (bounder == NULL || bounder->doIRect(dstM.fBounds))) {
        const SkIRect& cr = clipper.rect();
        do {
            blitter->blitMask(dstM, cr);
            clipper.next();
        } while (!clipper.done());
    }

    return true;
}

SkMaskFilter::BlurType SkMaskFilter::asABlur(BlurInfo*) {
    return kNone_BlurType;
}


