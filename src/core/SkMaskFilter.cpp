/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMaskFilterBase.h"

#include "SkAutoMalloc.h"
#include "SkBlitter.h"
#include "SkCachedData.h"
#include "SkCoverageModePriv.h"
#include "SkDraw.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRasterClip.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrTextureProxy.h"
#include "GrFragmentProcessor.h"
#endif

SkMaskFilterBase::NinePatch::~NinePatch() {
    if (fCache) {
        SkASSERT((const void*)fMask.fImage == fCache->data());
        fCache->unref();
    } else {
        SkMask::FreeImage(fMask.fImage);
    }
}

bool SkMaskFilterBase::filterMask(SkMask*, const SkMask&, const SkMatrix&,
                              SkIPoint*) const {
    return false;
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
        SkMask m;
        m.fImage = mask.getAddr8(mask.fBounds.left() + r.left() - outerR.left(),
                                 mask.fBounds.top() + cy);
        m.fBounds = r;
        m.fRowBytes = 0;    // so we repeat the scanline for our height
        m.fFormat = SkMask::kA8_Format;
        blitter->blitMask(m, r);
    }
    // right
    r.set(innerR.right(), innerR.top(), outerR.right(), innerR.bottom());
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
    if (path.isNestedFillRects(rects)) {
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
    SkASSERT(args.fLocalMatrix == nullptr);
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

bool SkMaskFilterBase::canFilterMaskGPU(const SkRRect& devRRect,
                                        const SkIRect& clipBounds,
                                        const SkMatrix& ctm,
                                        SkRect* maskRect) const {
    return false;
}

bool SkMaskFilterBase::directFilterMaskGPU(GrContext*,
                                           GrRenderTargetContext* renderTargetContext,
                                           GrPaint&&,
                                           const GrClip&,
                                           const SkMatrix& viewMatrix,
                                           const SkStrokeRec& strokeRec,
                                           const SkPath& path) const {
    return false;
}

bool SkMaskFilterBase::directFilterRRectMaskGPU(GrContext*,
                                                GrRenderTargetContext* renderTargetContext,
                                                GrPaint&&,
                                                const GrClip&,
                                                const SkMatrix& viewMatrix,
                                                const SkStrokeRec& strokeRec,
                                                const SkRRect& rrect,
                                                const SkRRect& devRRect) const {
    return false;
}

sk_sp<GrTextureProxy> SkMaskFilterBase::filterMaskGPU(GrContext*,
                                                      sk_sp<GrTextureProxy> srcProxy,
                                                      const SkMatrix& ctm,
                                                      const SkIRect& maskRect) const {
    return nullptr;
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

///////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> static inline T join(const T& a, const T& b) {
    T r = a;
    r.join(b);
    return r;
}
template <typename T> static inline T sect(const T& a, const T& b) {
    T r = a;
    return r.intersect(b) ? r : T::MakeEmpty();
}

class SkComposeMF : public SkMaskFilterBase {
public:
    SkComposeMF(sk_sp<SkMaskFilter> outer, sk_sp<SkMaskFilter> inner)
        : fOuter(std::move(outer))
        , fInner(std::move(inner))
    {
        SkASSERT(as_MFB(fOuter)->getFormat() == SkMask::kA8_Format);
        SkASSERT(as_MFB(fInner)->getFormat() == SkMask::kA8_Format);
    }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&, SkIPoint*) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        SkRect tmp;
        as_MFB(fInner)->computeFastBounds(src, &tmp);
        as_MFB(fOuter)->computeFastBounds(tmp, dst);
    }

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }
    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeMF)

private:
    sk_sp<SkMaskFilter> fOuter;
    sk_sp<SkMaskFilter> fInner;

    void flatten(SkWriteBuffer&) const override;

    friend class SkMaskFilter;

    typedef SkMaskFilterBase INHERITED;
};

bool SkComposeMF::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& ctm,
                             SkIPoint* margin) const {
    SkIPoint innerMargin;
    SkMask innerMask;

    if (!as_MFB(fInner)->filterMask(&innerMask, src, ctm, &innerMargin)) {
        return false;
    }
    if (!as_MFB(fOuter)->filterMask(dst, innerMask, ctm, margin)) {
        return false;
    }
    if (margin) {
        margin->fX += innerMargin.fX;
        margin->fY += innerMargin.fY;
    }
    sk_free(innerMask.fImage);
    return true;
}

void SkComposeMF::flatten(SkWriteBuffer & buffer) const {
    buffer.writeFlattenable(fOuter.get());
    buffer.writeFlattenable(fInner.get());
}

sk_sp<SkFlattenable> SkComposeMF::CreateProc(SkReadBuffer& buffer) {
    auto outer = buffer.readMaskFilter();
    auto inner = buffer.readMaskFilter();
    if (!buffer.validate(outer && inner)) {
        return nullptr;
    }
    return SkMaskFilter::MakeCompose(std::move(outer), std::move(inner));
}

#ifndef SK_IGNORE_TO_STRING
void SkComposeMF::toString(SkString* str) const {
    str->set("SkComposeMF:");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkCombineMF : public SkMaskFilterBase {
public:
    SkCombineMF(sk_sp<SkMaskFilter> dst, sk_sp<SkMaskFilter> src, SkCoverageMode mode)
        : fDst(std::move(dst))
        , fSrc(std::move(src))
        , fMode(mode)
    {
        SkASSERT(as_MFB(fSrc)->getFormat() == SkMask::kA8_Format);
        SkASSERT(as_MFB(fDst)->getFormat() == SkMask::kA8_Format);
    }

    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&, SkIPoint*) const override;

    void computeFastBounds(const SkRect& src, SkRect* dst) const override {
        SkRect srcR, dstR;
        as_MFB(fSrc)->computeFastBounds(src, &srcR);
        as_MFB(fDst)->computeFastBounds(src, &dstR);
        *dst = join(srcR, dstR);
    }

    SkMask::Format getFormat() const override { return SkMask::kA8_Format; }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkCombineMF)

private:
    sk_sp<SkMaskFilter> fDst;
    sk_sp<SkMaskFilter> fSrc;
    SkCoverageMode      fMode;

    void flatten(SkWriteBuffer&) const override;

    friend class SkMaskFilter;

    typedef SkMaskFilterBase INHERITED;
};

#include "SkSafeMath.h"

class DrawIntoMask : public SkDraw {
public:
    // we ignore the offset of the mask->fBounds
    DrawIntoMask(SkMask* mask) {
        int w = mask->fBounds.width();
        int h = mask->fBounds.height();
        size_t size = SkSafeMath::Mul(w, h);
        mask->fFormat = SkMask::kA8_Format;
        mask->fImage = SkMask::AllocImage(size, SkMask::kZeroInit_Alloc);
        mask->fRowBytes = w;

        SkAssertResult(fDst.reset(*mask));

        fMatrixStorage.reset();
        fMatrix = &fMatrixStorage;

        fRCStorage.setRect({ 0, 0, w, h });
        fRC = &fRCStorage;
    }

    void drawAsBitmap(const SkMask& m, const SkPaint& p) {
        SkBitmap b;
        b.installMaskPixels(m);
        this->drawSprite(b, m.fBounds.fLeft, m.fBounds.fTop, p);
    }

private:
    SkMatrix        fMatrixStorage;
    SkRasterClip    fRCStorage;
};

static SkIRect join(const SkIRect& src, const SkIRect& dst, SkCoverageMode mode) {
    switch (mode) {
        case SkCoverageMode::kUnion:                return join(src, dst);
        case SkCoverageMode::kIntersect:            return sect(src, dst);
        case SkCoverageMode::kDifference:           return src;
        case SkCoverageMode::kReverseDifference:    return dst;
        case SkCoverageMode::kXor:                  return join(src, dst);
    }
    // not reached
    return { 0, 0, 0, 0 };
}

bool SkCombineMF::filterMask(SkMask* dst, const SkMask& src, const SkMatrix& ctm,
                             SkIPoint* margin) const {
    SkIPoint srcP, dstP;
    SkMask srcM, dstM;

    if (!as_MFB(fSrc)->filterMask(&srcM, src, ctm, &srcP)) {
        return false;
    }
    if (!as_MFB(fDst)->filterMask(&dstM, src, ctm, &dstP)) {
        return false;
    }

    dst->fBounds = join(srcM.fBounds, dstM.fBounds, fMode);
    dst->fFormat = SkMask::kA8_Format;
    if (src.fImage == nullptr) {
        dst->fImage = nullptr;
        return true;
    }

    DrawIntoMask md(dst);
    SkPaint      p;

    p.setBlendMode(SkBlendMode::kSrc);
    dstM.fBounds.offset(-dst->fBounds.fLeft, -dst->fBounds.fTop);
    md.drawAsBitmap(dstM, p);
    p.setBlendMode(gUncorrelatedCoverageToBlend[static_cast<int>(fMode)]);
    srcM.fBounds.offset(-dst->fBounds.fLeft, -dst->fBounds.fTop);
    md.drawAsBitmap(srcM, p);

    sk_free(srcM.fImage);
    sk_free(dstM.fImage);
    return true;
}

void SkCombineMF::flatten(SkWriteBuffer & buffer) const {
    buffer.writeFlattenable(fDst.get());
    buffer.writeFlattenable(fSrc.get());
    buffer.write32(static_cast<uint32_t>(fMode));
}

sk_sp<SkFlattenable> SkCombineMF::CreateProc(SkReadBuffer& buffer) {
    auto dst = buffer.readMaskFilter();
    auto src = buffer.readMaskFilter();
    SkCoverageMode mode = buffer.read32LE(SkCoverageMode::kLast);
    if (!buffer.validate(dst && src)) {
        return nullptr;
    }
    return SkMaskFilter::MakeCombine(std::move(dst), std::move(src), mode);
}

#ifndef SK_IGNORE_TO_STRING
void SkCombineMF::toString(SkString* str) const {
    str->set("SkCombineMF:");
}
#endif

////////////////////////////////////////

sk_sp<SkMaskFilter> SkMaskFilter::MakeCompose(sk_sp<SkMaskFilter> outer,
                                              sk_sp<SkMaskFilter> inner) {
    if (!outer) {
        return inner;
    }
    if (!inner) {
        return outer;
    }
    if (as_MFB(inner)->getFormat() != SkMask::kA8_Format ||
        as_MFB(outer)->getFormat() != SkMask::kA8_Format) {
        return nullptr;
    }
    return sk_sp<SkMaskFilter>(new SkComposeMF(std::move(outer), std::move(inner)));
}

sk_sp<SkMaskFilter> SkMaskFilter::MakeCombine(sk_sp<SkMaskFilter> dst, sk_sp<SkMaskFilter> src,
                                              SkCoverageMode mode) {
    if (!dst) {
        return src;
    }
    if (!src) {
        return dst;
    }

    if (as_MFB(dst)->getFormat() != SkMask::kA8_Format ||
        as_MFB(src)->getFormat() != SkMask::kA8_Format) {
        return nullptr;
    }
    return sk_sp<SkMaskFilter>(new SkCombineMF(std::move(dst), std::move(src), mode));
}

void SkMaskFilter::InitializeFlattenables() {
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkComposeMF)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkCombineMF)
}

