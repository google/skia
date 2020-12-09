/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMaskFilterBase.h"

#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkDraw.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/text/GrSDFMaskFilter.h"
#endif

SkMaskFilterBase::NinePatch::~NinePatch() {
    if (fCache) {
        SkASSERT((const void*)fMask.fImage == fCache->data());
        fCache->unref();
    } else {
        SkMask::FreeImage(fMask.fImage);
    }
}

bool SkMaskFilterBase::asABlur(BlurRec*) const {
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
    innerR.setLTRB(outerR.left() + cx - mask.fBounds.left(),
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
    r.setLTRB(innerR.left(), outerR.top(), innerR.right(), innerR.top());
    if (r.intersect(clipR)) {
        int startY = std::max(0, r.top() - outerR.top());
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
    r.setLTRB(innerR.left(), innerR.bottom(), innerR.right(), outerR.bottom());
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
    r.setLTRB(outerR.left(), innerR.top(), innerR.left(), innerR.bottom());
    if (r.intersect(clipR)) {
        SkMask m;
        m.fImage = mask.getAddr8(mask.fBounds.left() + r.left() - outerR.left(),
                                 mask.fBounds.top() + cy);
        m.fBounds = r;
        m.fRowBytes = 0;    // so we repeat the scanline for our height
        m.fFormat = SkMask::kA8_Format;
        blitter->blitMask(m, r);
    }
    // right
    r.setLTRB(innerR.right(), innerR.top(), outerR.right(), innerR.bottom());
    if (r.intersect(clipR)) {
        SkMask m;
        m.fImage = mask.getAddr8(mask.fBounds.right() - outerR.right() + r.left(),
                                 mask.fBounds.top() + cy);
        m.fBounds = r;
        m.fRowBytes = 0;    // so we repeat the scanline for our height
        m.fFormat = SkMask::kA8_Format;
        blitter->blitMask(m, r);
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

bool SkMaskFilterBase::filterRRect(const SkRRect& devRRect, const SkMatrix& matrix,
                                   const SkRasterClip& clip, SkBlitter* blitter) const {
    // Attempt to speed up drawing by creating a nine patch. If a nine patch
    // cannot be used, return false to allow our caller to recover and perform
    // the drawing another way.
    NinePatch patch;
    patch.fMask.fImage = nullptr;
    if (kTrue_FilterReturn != this->filterRRectToNine(devRRect, matrix,
                                                      clip.getBounds(),
                                                      &patch)) {
        SkASSERT(nullptr == patch.fMask.fImage);
        return false;
    }
    draw_nine(patch.fMask, patch.fOuterRect, patch.fCenter, true, clip, blitter);
    return true;
}

bool SkMaskFilterBase::filterPath(const SkPath& devPath, const SkMatrix& matrix,
                                  const SkRasterClip& clip, SkBlitter* blitter,
                                  SkStrokeRec::InitStyle style) const {
    SkRect rects[2];
    int rectCount = 0;
    if (SkStrokeRec::kFill_InitStyle == style) {
        rectCount = countNestedRects(devPath, rects);
    }
    if (rectCount > 0) {
        NinePatch patch;

        switch (this->filterRectsToNine(rects, rectCount, matrix, clip.getBounds(), &patch)) {
            case kFalse_FilterReturn:
                SkASSERT(nullptr == patch.fMask.fImage);
                return false;

            case kTrue_FilterReturn:
                draw_nine(patch.fMask, patch.fOuterRect, patch.fCenter, 1 == rectCount, clip,
                          blitter);
                return true;

            case kUnimplemented_FilterReturn:
                SkASSERT(nullptr == patch.fMask.fImage);
                // fall out
                break;
        }
    }

    SkMask  srcM, dstM;

#if defined(SK_BUILD_FOR_FUZZER)
    if (devPath.countVerbs() > 1000 || devPath.countPoints() > 1000) {
        return false;
    }
#endif
    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), this, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode,
                            style)) {
        return false;
    }
    SkAutoMaskFreeImage autoSrc(srcM.fImage);

    if (!this->filterMask(&dstM, srcM, matrix, nullptr)) {
        return false;
    }
    SkAutoMaskFreeImage autoDst(dstM.fImage);

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

SkMaskFilterBase::FilterReturn
SkMaskFilterBase::filterRRectToNine(const SkRRect&, const SkMatrix&,
                                    const SkIRect& clipBounds, NinePatch*) const {
    return kUnimplemented_FilterReturn;
}

SkMaskFilterBase::FilterReturn
SkMaskFilterBase::filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                    const SkIRect& clipBounds, NinePatch*) const {
    return kUnimplemented_FilterReturn;
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor>
SkMaskFilterBase::asFragmentProcessor(const GrFPArgs& args) const {
    auto fp = this->onAsFragmentProcessor(args);
    if (fp) {
        SkASSERT(this->hasFragmentProcessor());
    } else {
        SkASSERT(!this->hasFragmentProcessor());
    }
    return fp;
}
bool SkMaskFilterBase::hasFragmentProcessor() const {
    return this->onHasFragmentProcessor();
}

std::unique_ptr<GrFragmentProcessor>
SkMaskFilterBase::onAsFragmentProcessor(const GrFPArgs&) const {
    return nullptr;
}
bool SkMaskFilterBase::onHasFragmentProcessor() const { return false; }

bool SkMaskFilterBase::canFilterMaskGPU(const GrStyledShape& shape,
                                        const SkIRect& devSpaceShapeBounds,
                                        const SkIRect& clipBounds,
                                        const SkMatrix& ctm,
                                        SkIRect* maskRect) const {
    return false;
}

bool SkMaskFilterBase::directFilterMaskGPU(GrRecordingContext*,
                                           GrSurfaceDrawContext*,
                                           GrPaint&&,
                                           const GrClip*,
                                           const SkMatrix& viewMatrix,
                                           const GrStyledShape&) const {
    return false;
}

GrSurfaceProxyView SkMaskFilterBase::filterMaskGPU(GrRecordingContext*,
                                                   GrSurfaceProxyView view,
                                                   GrColorType srcColorType,
                                                   SkAlphaType srcAlphaType,
                                                   const SkMatrix& ctm,
                                                   const SkIRect& maskRect) const {
    return {};
}
#endif

void SkMaskFilterBase::computeFastBounds(const SkRect& src, SkRect* dst) const {
    SkMask  srcM, dstM;

    srcM.fBounds = src.roundOut();
    srcM.fRowBytes = 0;
    srcM.fFormat = SkMask::kA8_Format;

    SkIPoint margin;    // ignored
    if (this->filterMask(&dstM, srcM, SkMatrix::I(), &margin)) {
        dst->set(dstM.fBounds);
    } else {
        dst->set(srcM.fBounds);
    }
}

void SkMaskFilter::RegisterFlattenables() {
    sk_register_blur_maskfilter_createproc();
#if SK_SUPPORT_GPU
    gr_register_sdf_maskfilter_createproc();
#endif
}
