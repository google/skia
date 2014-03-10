
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
#include "SkRRect.h"
#include "SkTypes.h"

#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "SkGr.h"
#include "SkGrPixelRef.h"
#endif

bool SkMaskFilter::filterMask(SkMask*, const SkMask&, const SkMatrix&,
                              SkIPoint*) const {
    return false;
}

static void extractMaskSubset(const SkMask& src, SkMask* dst) {
    SkASSERT(src.fBounds.contains(dst->fBounds));

    const int dx = dst->fBounds.left() - src.fBounds.left();
    const int dy = dst->fBounds.top() - src.fBounds.top();
    dst->fImage = src.fImage + dy * src.fRowBytes + dx;
    dst->fRowBytes = src.fRowBytes;
    dst->fFormat = src.fFormat;
}

static void blitClippedMask(SkBlitter* blitter, const SkMask& mask,
                            const SkIRect& bounds, const SkIRect& clipR) {
    SkIRect r;
    if (r.intersect(bounds, clipR)) {
        blitter->blitMask(mask, r);
    }
}

static void blitClippedRect(SkBlitter* blitter, const SkIRect& rect, const SkIRect& clipR) {
    SkIRect r;
    if (r.intersect(rect, clipR)) {
        blitter->blitRect(r.left(), r.top(), r.width(), r.height());
    }
}

#if 0
static void dump(const SkMask& mask) {
    for (int y = mask.fBounds.top(); y < mask.fBounds.bottom(); ++y) {
        for (int x = mask.fBounds.left(); x < mask.fBounds.right(); ++x) {
            SkDebugf("%02X", *mask.getAddr8(x, y));
        }
        SkDebugf("\n");
    }
    SkDebugf("\n");
}
#endif

static void draw_nine_clipped(const SkMask& mask, const SkIRect& outerR,
                              const SkIPoint& center, bool fillCenter,
                              const SkIRect& clipR, SkBlitter* blitter) {
    int cx = center.x();
    int cy = center.y();
    SkMask m;

    // top-left
    m.fBounds = mask.fBounds;
    m.fBounds.fRight = cx;
    m.fBounds.fBottom = cy;
    if (m.fBounds.width() > 0 && m.fBounds.height() > 0) {
        extractMaskSubset(mask, &m);
        m.fBounds.offsetTo(outerR.left(), outerR.top());
        blitClippedMask(blitter, m, m.fBounds, clipR);
    }

    // top-right
    m.fBounds = mask.fBounds;
    m.fBounds.fLeft = cx + 1;
    m.fBounds.fBottom = cy;
    if (m.fBounds.width() > 0 && m.fBounds.height() > 0) {
        extractMaskSubset(mask, &m);
        m.fBounds.offsetTo(outerR.right() - m.fBounds.width(), outerR.top());
        blitClippedMask(blitter, m, m.fBounds, clipR);
    }

    // bottom-left
    m.fBounds = mask.fBounds;
    m.fBounds.fRight = cx;
    m.fBounds.fTop = cy + 1;
    if (m.fBounds.width() > 0 && m.fBounds.height() > 0) {
        extractMaskSubset(mask, &m);
        m.fBounds.offsetTo(outerR.left(), outerR.bottom() - m.fBounds.height());
        blitClippedMask(blitter, m, m.fBounds, clipR);
    }

    // bottom-right
    m.fBounds = mask.fBounds;
    m.fBounds.fLeft = cx + 1;
    m.fBounds.fTop = cy + 1;
    if (m.fBounds.width() > 0 && m.fBounds.height() > 0) {
        extractMaskSubset(mask, &m);
        m.fBounds.offsetTo(outerR.right() - m.fBounds.width(),
                           outerR.bottom() - m.fBounds.height());
        blitClippedMask(blitter, m, m.fBounds, clipR);
    }

    SkIRect innerR;
    innerR.set(outerR.left() + cx - mask.fBounds.left(),
               outerR.top() + cy - mask.fBounds.top(),
               outerR.right() + (cx + 1 - mask.fBounds.right()),
               outerR.bottom() + (cy + 1 - mask.fBounds.bottom()));
    if (fillCenter) {
        blitClippedRect(blitter, innerR, clipR);
    }

    const int innerW = innerR.width();
    size_t storageSize = (innerW + 1) * (sizeof(int16_t) + sizeof(uint8_t));
    SkAutoSMalloc<4*1024> storage(storageSize);
    int16_t* runs = (int16_t*)storage.get();
    uint8_t* alpha = (uint8_t*)(runs + innerW + 1);

    SkIRect r;
    // top
    r.set(innerR.left(), outerR.top(), innerR.right(), innerR.top());
    if (r.intersect(clipR)) {
        int startY = SkMax32(0, r.top() - outerR.top());
        int stopY = startY + r.height();
        int width = r.width();
        for (int y = startY; y < stopY; ++y) {
            runs[0] = width;
            runs[width] = 0;
            alpha[0] = *mask.getAddr8(cx, mask.fBounds.top() + y);
            blitter->blitAntiH(r.left(), outerR.top() + y, alpha, runs);
        }
    }
    // bottom
    r.set(innerR.left(), innerR.bottom(), innerR.right(), outerR.bottom());
    if (r.intersect(clipR)) {
        int startY = outerR.bottom() - r.bottom();
        int stopY = startY + r.height();
        int width = r.width();
        for (int y = startY; y < stopY; ++y) {
            runs[0] = width;
            runs[width] = 0;
            alpha[0] = *mask.getAddr8(cx, mask.fBounds.bottom() - y - 1);
            blitter->blitAntiH(r.left(), outerR.bottom() - y - 1, alpha, runs);
        }
    }
    // left
    r.set(outerR.left(), innerR.top(), innerR.left(), innerR.bottom());
    if (r.intersect(clipR)) {
        int startX = r.left() - outerR.left();
        int stopX = startX + r.width();
        int height = r.height();
        for (int x = startX; x < stopX; ++x) {
            blitter->blitV(outerR.left() + x, r.top(), height,
                           *mask.getAddr8(mask.fBounds.left() + x, mask.fBounds.top() + cy));
        }
    }
    // right
    r.set(innerR.right(), innerR.top(), outerR.right(), innerR.bottom());
    if (r.intersect(clipR)) {
        int startX = outerR.right() - r.right();
        int stopX = startX + r.width();
        int height = r.height();
        for (int x = startX; x < stopX; ++x) {
            blitter->blitV(outerR.right() - x - 1, r.top(), height,
                           *mask.getAddr8(mask.fBounds.right() - x - 1, mask.fBounds.top() + cy));
        }
    }
}

static void draw_nine(const SkMask& mask, const SkIRect& outerR,
                      const SkIPoint& center, bool fillCenter,
                      const SkRasterClip& clip, SkBounder* bounder,
                      SkBlitter* blitter) {
    // if we get here, we need to (possibly) resolve the clip and blitter
    SkAAClipBlitterWrapper wrapper(clip, blitter);
    blitter = wrapper.getBlitter();

    SkRegion::Cliperator clipper(wrapper.getRgn(), outerR);

    if (!clipper.done() && (!bounder || bounder->doIRect(outerR))) {
        const SkIRect& cr = clipper.rect();
        do {
            draw_nine_clipped(mask, outerR, center, fillCenter, cr, blitter);
            clipper.next();
        } while (!clipper.done());
    }
}

static int countNestedRects(const SkPath& path, SkRect rects[2]) {
    if (path.isNestedRects(rects)) {
        return 2;
    }
    return path.isRect(&rects[0]);
}

bool SkMaskFilter::filterRRect(const SkRRect& devRRect, const SkMatrix& matrix,
                               const SkRasterClip& clip, SkBounder* bounder,
                               SkBlitter* blitter, SkPaint::Style style) const {
    // Attempt to speed up drawing by creating a nine patch. If a nine patch
    // cannot be used, return false to allow our caller to recover and perform
    // the drawing another way.
    NinePatch patch;
    patch.fMask.fImage = NULL;
    if (kTrue_FilterReturn != this->filterRRectToNine(devRRect, matrix,
                                                      clip.getBounds(),
                                                      &patch)) {
        SkASSERT(NULL == patch.fMask.fImage);
        return false;
    }
    draw_nine(patch.fMask, patch.fOuterRect, patch.fCenter, true, clip,
              bounder, blitter);
    SkMask::FreeImage(patch.fMask.fImage);
    return true;
}

bool SkMaskFilter::filterPath(const SkPath& devPath, const SkMatrix& matrix,
                              const SkRasterClip& clip, SkBounder* bounder,
                              SkBlitter* blitter, SkPaint::Style style) const {
    SkRect rects[2];
    int rectCount = 0;
    if (SkPaint::kFill_Style == style) {
        rectCount = countNestedRects(devPath, rects);
    }
    if (rectCount > 0) {
        NinePatch patch;

        patch.fMask.fImage = NULL;
        switch (this->filterRectsToNine(rects, rectCount, matrix,
                                        clip.getBounds(), &patch)) {
            case kFalse_FilterReturn:
                SkASSERT(NULL == patch.fMask.fImage);
                return false;

            case kTrue_FilterReturn:
                draw_nine(patch.fMask, patch.fOuterRect, patch.fCenter,
                          1 == rectCount, clip, bounder, blitter);
                SkMask::FreeImage(patch.fMask.fImage);
                return true;

            case kUnimplemented_FilterReturn:
                SkASSERT(NULL == patch.fMask.fImage);
                // fall through
                break;
        }
    }

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

SkMaskFilter::FilterReturn
SkMaskFilter::filterRRectToNine(const SkRRect&, const SkMatrix&,
                                const SkIRect& clipBounds, NinePatch*) const {
    return kUnimplemented_FilterReturn;
}

SkMaskFilter::FilterReturn
SkMaskFilter::filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                const SkIRect& clipBounds, NinePatch*) const {
    return kUnimplemented_FilterReturn;
}

#if SK_SUPPORT_GPU
bool SkMaskFilter::asNewEffect(GrEffectRef** effect, GrTexture*, const SkMatrix&) const {
    return false;
}

bool SkMaskFilter::canFilterMaskGPU(const SkRect& devBounds,
                                    const SkIRect& clipBounds,
                                    const SkMatrix& ctm,
                                    SkRect* maskRect) const {
    return false;
}

 bool SkMaskFilter::directFilterMaskGPU(GrContext* context,
                                        GrPaint* grp,
                                        const SkStrokeRec& strokeRec,
                                        const SkPath& path) const {
    return false;
}


bool SkMaskFilter::directFilterRRectMaskGPU(GrContext* context,
                                            GrPaint* grp,
                                            const SkStrokeRec& strokeRec,
                                            const SkRRect& rrect) const {
    return false;
}

bool SkMaskFilter::filterMaskGPU(GrTexture* src,
                                 const SkMatrix& ctm,
                                 const SkRect& maskRect,
                                 GrTexture** result,
                                 bool canOverwriteSrc) const {
    return false;
}
#endif

void SkMaskFilter::computeFastBounds(const SkRect& src, SkRect* dst) const {
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
