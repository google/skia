
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
#include "SkRasterClip.h"

bool SkMaskFilter::filterMask(SkMask*, const SkMask&, const SkMatrix&,
                              SkIPoint*) {
    return false;
}

bool SkMaskFilter::filterPath(const SkPath& devPath, const SkMatrix& matrix,
                              const SkRasterClip& clip, SkBounder* bounder,
                              SkBlitter* blitter) {
    SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), this, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
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


