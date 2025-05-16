/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/core/SkMaskFilterBase.h"

#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkDraw.h"
#include "src/core/SkMask.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

class SkPaint;
class SkRRect;

SkMaskFilterBase::NinePatch::~NinePatch() {
    if (fCache) {
        SkASSERT((const void*)fMask.fImage == fCache->data());
        fCache->unref();
    } else {
        // fMask is about to be destroyed and "owns" its fImage.
        SkMaskBuilder::FreeImage(const_cast<uint8_t*>(fMask.fImage));
    }
}

bool SkMaskFilterBase::asABlur(BlurRec*) const {
    return false;
}

std::pair<sk_sp<SkImageFilter>, bool> SkMaskFilterBase::asImageFilter(const SkMatrix& ctm,
                                                                      const SkPaint& paint) const {
    return std::make_pair(nullptr, false);
}

static SkMask extract_mask_subset(const SkMask& src, SkIRect bounds, int32_t newX, int32_t newY) {
    SkASSERT(src.fBounds.contains(bounds));

    const int dx = bounds.left() - src.fBounds.left();
    const int dy = bounds.top() - src.fBounds.top();
    bounds.offsetTo(newX, newY);
    return SkMask(src.fImage + dy * src.fRowBytes + dx,
                  bounds,
                  src.fRowBytes,
                  src.fFormat);
}

static void blit_clipped_mask(SkBlitter* blitter, const SkMask& mask,
                            const SkIRect& bounds, const SkIRect& clipR) {
    SkIRect r;
    if (r.intersect(bounds, clipR)) {
        blitter->blitMask(mask, r);
    }
}

static void blit_clipped_rect(SkBlitter* blitter, const SkIRect& rect, const SkIRect& clipR) {
    SkIRect r;
    if (r.intersect(rect, clipR)) {
        blitter->blitRect(r.left(), r.top(), r.width(), r.height());
    }
}

static void draw_nine_clipped(const SkMask& mask, const SkIRect& outerR,
                              const SkIPoint& center, bool fillCenter,
                              const SkIRect& clipR, SkBlitter* blitter) {
    const int cx = center.x();
    const int cy = center.y();
    SkIRect bounds;

    // top-left
    bounds = mask.fBounds;
    bounds.fRight = cx;
    bounds.fBottom = cy;
    if (bounds.width() > 0 && bounds.height() > 0) {
        SkMask m = extract_mask_subset(mask, bounds, outerR.left(), outerR.top());
        blit_clipped_mask(blitter, m, m.fBounds, clipR);
    }

    // top-right
    bounds = mask.fBounds;
    bounds.fLeft = cx + 1;
    bounds.fBottom = cy;
    if (bounds.width() > 0 && bounds.height() > 0) {
        SkMask m = extract_mask_subset(mask, bounds, outerR.right() - bounds.width(), outerR.top());
        blit_clipped_mask(blitter, m, m.fBounds, clipR);
    }

    // bottom-left
    bounds = mask.fBounds;
    bounds.fRight = cx;
    bounds.fTop = cy + 1;
    if (bounds.width() > 0 && bounds.height() > 0) {
        SkMask m = extract_mask_subset(mask, bounds, outerR.left(), outerR.bottom() - bounds.height());
        blit_clipped_mask(blitter, m, m.fBounds, clipR);
    }

    // bottom-right
    bounds = mask.fBounds;
    bounds.fLeft = cx + 1;
    bounds.fTop = cy + 1;
    if (bounds.width() > 0 && bounds.height() > 0) {
        SkMask m = extract_mask_subset(mask, bounds, outerR.right() - bounds.width(),
                                                   outerR.bottom() - bounds.height());
        blit_clipped_mask(blitter, m, m.fBounds, clipR);
    }

    SkIRect innerR;
    innerR.setLTRB(outerR.left() + cx - mask.fBounds.left(),
                   outerR.top() + cy - mask.fBounds.top(),
                   outerR.right() + (cx + 1 - mask.fBounds.right()),
                   outerR.bottom() + (cy + 1 - mask.fBounds.bottom()));
    if (fillCenter) {
        blit_clipped_rect(blitter, innerR, clipR);
    }

    const int innerW = innerR.width();
    size_t storageSize = (innerW + 1) * (sizeof(int16_t) + sizeof(uint8_t));
    SkAutoSMalloc<4*1024> storage(storageSize);
    int16_t* runs = (int16_t*)storage.get();
    uint8_t* alpha = (uint8_t*)(runs + innerW + 1);

    SkIRect r;
    // top
    r.setLTRB(innerR.left(), outerR.top(), innerR.right(), innerR.top());
    if (r.intersect(clipR)) {
        const int startY = std::max(0, r.top() - outerR.top());
        const int stopY = startY + r.height();
        const int width = r.width();
        runs[0] = width;
        runs[width] = 0;
        for (int y = startY; y < stopY; ++y) {
            alpha[0] = *mask.getAddr8(cx, mask.fBounds.top() + y);
            blitter->blitAntiH(r.left(), outerR.top() + y, alpha, runs);
        }
    }
    // bottom
    r.setLTRB(innerR.left(), innerR.bottom(), innerR.right(), outerR.bottom());
    if (r.intersect(clipR)) {
        const int startY = outerR.bottom() - r.bottom();
        const int stopY = startY + r.height();
        const int width = r.width();
        runs[0] = width;
        runs[width] = 0;
        for (int y = startY; y < stopY; ++y) {
            alpha[0] = *mask.getAddr8(cx, mask.fBounds.bottom() - y - 1);
            blitter->blitAntiH(r.left(), outerR.bottom() - y - 1, alpha, runs);
        }
    }
    // left
    r.setLTRB(outerR.left(), innerR.top(), innerR.left(), innerR.bottom());
    if (r.intersect(clipR)) {
        SkMask leftMask(mask.getAddr8(mask.fBounds.left() + r.left() - outerR.left(),
                                      mask.fBounds.top() + cy),
                        r,
                        0,    // so we repeat the scanline for our height
                        SkMask::kA8_Format);
        blitter->blitMask(leftMask, r);
    }
    // right
    r.setLTRB(innerR.right(), innerR.top(), outerR.right(), innerR.bottom());
    if (r.intersect(clipR)) {
        SkMask rightMask(mask.getAddr8(mask.fBounds.right() - outerR.right() + r.left(),
                                       mask.fBounds.top() + cy),
                         r,
                         0,    // so we repeat the scanline for our height
                         SkMask::kA8_Format);
        blitter->blitMask(rightMask, r);
    }
}

static void draw_nine(const SkMask& mask, const SkIRect& outerR, const SkIPoint& center,
                      bool fillCenter, const SkRasterClip& clip, SkBlitter* blitter) {
    // if we get here, we need to (possibly) resolve the clip and blitter
    SkAAClipBlitterWrapper wrapper(clip, blitter);
    blitter = wrapper.getBlitter();

    SkRegion::Cliperator clipper(wrapper.getRgn(), outerR);

    if (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        do {
            draw_nine_clipped(mask, outerR, center, fillCenter, cr, blitter);
            clipper.next();
        } while (!clipper.done());
    }
}

static int countNestedRects(const SkPath& path, SkRect rects[2]) {
    if (SkPathPriv::IsNestedFillRects(path, rects)) {
        return 2;
    }
    return path.isRect(&rects[0]);
}

bool SkMaskFilterBase::filterRRect(const SkRRect& devRRect,
                                   const SkMatrix& matrix,
                                   const SkRasterClip& clip,
                                   SkBlitter* blitter,
                                   SkResourceCache* cache) const {
    // Attempt to speed up drawing by creating a nine patch. If a nine patch
    // cannot be used, return false to allow our caller to recover and perform
    // the drawing another way.
    std::optional<NinePatch> patch =
            this->filterRRectToNine(devRRect, matrix, clip.getBounds(), cache);

    if (!patch.has_value()) {
        return false;
    }
    draw_nine(patch->fMask, patch->fOuterRect, patch->fCenter, true, clip, blitter);
    return true;
}

bool SkMaskFilterBase::filterPath(const SkPath& devPath,
                                  const SkMatrix& matrix,
                                  const SkRasterClip& clip,
                                  SkBlitter* blitter,
                                  SkStrokeRec::InitStyle style,
                                  SkResourceCache* cache) const {
    SkRect rects[2];
    int rectCount = 0;
    if (SkStrokeRec::kFill_InitStyle == style) {
        rectCount = countNestedRects(devPath, rects);
    }
    if (rectCount > 0) {
        std::optional<NinePatch> patch;

        switch (this->filterRectsToNine(
                SkSpan(rects, rectCount), matrix, clip.getBounds(), &patch, cache)) {
            case FilterReturn::kFalse:
                SkASSERT(!patch.has_value());
                return false;

            case FilterReturn::kTrue:
                draw_nine(patch->fMask, patch->fOuterRect, patch->fCenter, 1 == rectCount, clip,
                          blitter);
                return true;

            case FilterReturn::kUnimplemented:
                SkASSERT(!patch.has_value());
                // fall out
                break;
        }
    }

    SkMaskBuilder srcM, dstM;

#if defined(SK_BUILD_FOR_FUZZER)
    if (devPath.countVerbs() > 1000 || devPath.countPoints() > 1000) {
        return false;
    }
#endif
    if (!SkDraw::DrawToMask(devPath, clip.getBounds(), this, &matrix, &srcM,
                            SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode,
                            style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.image());

    if (!this->filterMask(&dstM, srcM, matrix, nullptr)) {
        return false;
    }
    SkAutoMaskFreeImage autoDst(dstM.image());

    // if we get here, we need to (possibly) resolve the clip and blitter
    SkAAClipBlitterWrapper wrapper(clip, blitter);
    blitter = wrapper.getBlitter();

    SkRegion::Cliperator clipper(wrapper.getRgn(), dstM.fBounds);

    if (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        do {
            blitter->blitMask(dstM, cr);
            clipper.next();
        } while (!clipper.done());
    }

    return true;
}

std::optional<SkMaskFilterBase::NinePatch> SkMaskFilterBase::filterRRectToNine(
        const SkRRect&, const SkMatrix&, const SkIRect&, SkResourceCache*) const {
    return std::nullopt;
}

SkMaskFilterBase::FilterReturn SkMaskFilterBase::filterRectsToNine(SkSpan<const SkRect>,
                                                                   const SkMatrix&,
                                                                   const SkIRect&,
                                                                   std::optional<NinePatch>*,
                                                                   SkResourceCache*) const {
    return FilterReturn::kUnimplemented;
}

void SkMaskFilterBase::computeFastBounds(const SkRect& src, SkRect* dst) const {
    SkMask srcM(nullptr, src.roundOut(), 0, SkMask::kA8_Format);
    SkMaskBuilder dstM;

    SkIPoint margin;    // ignored
    if (this->filterMask(&dstM, srcM, SkMatrix::I(), &margin)) {
        dst->set(dstM.fBounds);
    } else {
        dst->set(srcM.fBounds);
    }
}
