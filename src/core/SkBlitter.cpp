/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlitter.h"
#include "SkAntiRun.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkString.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkXfermodeInterpretation.h"

// define this for testing srgb blits
//#define SK_FORCE_PM4f_FOR_L32_BLITS

SkBlitter::~SkBlitter() {}

bool SkBlitter::isNullBlitter() const { return false; }

const SkPixmap* SkBlitter::justAnOpaqueColor(uint32_t* value) {
    return nullptr;
}

/*
void SkBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("unimplemented");
}


void SkBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                          const int16_t runs[]) {
    SkDEBUGFAIL("unimplemented");
}
 */

void SkBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (alpha == 255) {
        this->blitRect(x, y, 1, height);
    } else {
        int16_t runs[2];
        runs[0] = 1;
        runs[1] = 0;

        while (--height >= 0) {
            this->blitAntiH(x, y++, &alpha, runs);
        }
    }
}

void SkBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(width > 0);
    while (--height >= 0) {
        this->blitH(x, y++, width);
    }
}

/// Default implementation doesn't check for easy optimizations
/// such as alpha == 255; also uses blitV(), which some subclasses
/// may not support.
void SkBlitter::blitAntiRect(int x, int y, int width, int height,
                             SkAlpha leftAlpha, SkAlpha rightAlpha) {
    if (leftAlpha > 0) { // we may send in x = -1 with leftAlpha = 0
        this->blitV(x, y, height, leftAlpha);
    }
    x++;
    if (width > 0) {
        this->blitRect(x, y, width, height);
        x += width;
    }
    if (rightAlpha > 0) {
        this->blitV(x, y, height, rightAlpha);
    }
}

//////////////////////////////////////////////////////////////////////////////

static inline void bits_to_runs(SkBlitter* blitter, int x, int y,
                                const uint8_t bits[],
                                uint8_t left_mask, ptrdiff_t rowBytes,
                                uint8_t right_mask) {
    int inFill = 0;
    int pos = 0;

    while (--rowBytes >= 0) {
        uint8_t b = *bits++ & left_mask;
        if (rowBytes == 0) {
            b &= right_mask;
        }

        for (uint8_t test = 0x80U; test != 0; test >>= 1) {
            if (b & test) {
                if (!inFill) {
                    pos = x;
                    inFill = true;
                }
            } else {
                if (inFill) {
                    blitter->blitH(pos, y, x - pos);
                    inFill = false;
                }
            }
            x += 1;
        }
        left_mask = 0xFFU;
    }

    // final cleanup
    if (inFill) {
        blitter->blitH(pos, y, x - pos);
    }
}

// maskBitCount is the number of 1's to place in the mask. It must be in the range between 1 and 8.
static uint8_t generate_right_mask(int maskBitCount) {
    return static_cast<uint8_t>(0xFF00U >> maskBitCount);
}

void SkBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (mask.fFormat == SkMask::kLCD16_Format) {
        return; // needs to be handled by subclass
    }

    if (mask.fFormat == SkMask::kBW_Format) {
        int cx = clip.fLeft;
        int cy = clip.fTop;
        int maskLeft = mask.fBounds.fLeft;
        int maskRowBytes = mask.fRowBytes;
        int height = clip.height();

        const uint8_t* bits = mask.getAddr1(cx, cy);

        SkDEBUGCODE(const uint8_t* endOfImage =
            mask.fImage + (mask.fBounds.height() - 1) * maskRowBytes
            + ((mask.fBounds.width() + 7) >> 3));

        if (cx == maskLeft && clip.fRight == mask.fBounds.fRight) {
            while (--height >= 0) {
                int affectedRightBit = mask.fBounds.width() - 1;
                ptrdiff_t rowBytes = (affectedRightBit >> 3) + 1;
                SkASSERT(bits + rowBytes <= endOfImage);
                U8CPU rightMask = generate_right_mask((affectedRightBit & 7) + 1);
                bits_to_runs(this, cx, cy, bits, 0xFF, rowBytes, rightMask);
                bits += maskRowBytes;
                cy += 1;
            }
        } else {
            // Bits is calculated as the offset into the mask at the point {cx, cy} therefore, all
            // addressing into the bit mask is relative to that point. Since this is an address
            // calculated from a arbitrary bit in that byte, calculate the left most bit.
            int bitsLeft = cx - ((cx - maskLeft) & 7);

            // Everything is relative to the bitsLeft.
            int leftEdge = cx - bitsLeft;
            SkASSERT(leftEdge >= 0);
            int rightEdge = clip.fRight - bitsLeft;
            SkASSERT(rightEdge > leftEdge);

            // Calculate left byte and mask
            const uint8_t* leftByte = bits;
            U8CPU leftMask = 0xFFU >> (leftEdge & 7);

            // Calculate right byte and mask
            int affectedRightBit = rightEdge - 1;
            const uint8_t* rightByte = bits + (affectedRightBit >> 3);
            U8CPU rightMask = generate_right_mask((affectedRightBit & 7) + 1);

            // leftByte and rightByte are byte locations therefore, to get a count of bytes the
            // code must add one.
            ptrdiff_t rowBytes = rightByte - leftByte + 1;

            while (--height >= 0) {
                SkASSERT(bits + rowBytes <= endOfImage);
                bits_to_runs(this, bitsLeft, cy, bits, leftMask, rowBytes, rightMask);
                bits += maskRowBytes;
                cy += 1;
            }
        }
    } else {
        int                         width = clip.width();
        SkAutoSTMalloc<64, int16_t> runStorage(width + 1);
        int16_t*                    runs = runStorage.get();
        const uint8_t*              aa = mask.getAddr8(clip.fLeft, clip.fTop);

        sk_memset16((uint16_t*)runs, 1, width);
        runs[width] = 0;

        int height = clip.height();
        int y = clip.fTop;
        while (--height >= 0) {
            this->blitAntiH(clip.fLeft, y, aa, runs);
            aa += mask.fRowBytes;
            y += 1;
        }
    }
}

/////////////////////// these guys are not virtual, just a helpers

void SkBlitter::blitMaskRegion(const SkMask& mask, const SkRegion& clip) {
    if (clip.quickReject(mask.fBounds)) {
        return;
    }

    SkRegion::Cliperator clipper(clip, mask.fBounds);

    while (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        this->blitMask(mask, cr);
        clipper.next();
    }
}

void SkBlitter::blitRectRegion(const SkIRect& rect, const SkRegion& clip) {
    SkRegion::Cliperator clipper(clip, rect);

    while (!clipper.done()) {
        const SkIRect& cr = clipper.rect();
        this->blitRect(cr.fLeft, cr.fTop, cr.width(), cr.height());
        clipper.next();
    }
}

void SkBlitter::blitRegion(const SkRegion& clip) {
    SkRegion::Iterator iter(clip);

    while (!iter.done()) {
        const SkIRect& cr = iter.rect();
        this->blitRect(cr.fLeft, cr.fTop, cr.width(), cr.height());
        iter.next();
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkNullBlitter::blitH(int x, int y, int width) {}

void SkNullBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                              const int16_t runs[]) {}

void SkNullBlitter::blitV(int x, int y, int height, SkAlpha alpha) {}

void SkNullBlitter::blitRect(int x, int y, int width, int height) {}

void SkNullBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {}

const SkPixmap* SkNullBlitter::justAnOpaqueColor(uint32_t* value) {
    return nullptr;
}

bool SkNullBlitter::isNullBlitter() const { return true; }

///////////////////////////////////////////////////////////////////////////////

static int compute_anti_width(const int16_t runs[]) {
    int width = 0;

    for (;;) {
        int count = runs[0];

        SkASSERT(count >= 0);
        if (count == 0) {
            break;
        }
        width += count;
        runs += count;
    }
    return width;
}

static inline bool y_in_rect(int y, const SkIRect& rect) {
    return (unsigned)(y - rect.fTop) < (unsigned)rect.height();
}

static inline bool x_in_rect(int x, const SkIRect& rect) {
    return (unsigned)(x - rect.fLeft) < (unsigned)rect.width();
}

void SkRectClipBlitter::blitH(int left, int y, int width) {
    SkASSERT(width > 0);

    if (!y_in_rect(y, fClipRect)) {
        return;
    }

    int right = left + width;

    if (left < fClipRect.fLeft) {
        left = fClipRect.fLeft;
    }
    if (right > fClipRect.fRight) {
        right = fClipRect.fRight;
    }

    width = right - left;
    if (width > 0) {
        fBlitter->blitH(left, y, width);
    }
}

void SkRectClipBlitter::blitAntiH(int left, int y, const SkAlpha aa[],
                                  const int16_t runs[]) {
    if (!y_in_rect(y, fClipRect) || left >= fClipRect.fRight) {
        return;
    }

    int x0 = left;
    int x1 = left + compute_anti_width(runs);

    if (x1 <= fClipRect.fLeft) {
        return;
    }

    SkASSERT(x0 < x1);
    if (x0 < fClipRect.fLeft) {
        int dx = fClipRect.fLeft - x0;
        SkAlphaRuns::BreakAt((int16_t*)runs, (uint8_t*)aa, dx);
        runs += dx;
        aa += dx;
        x0 = fClipRect.fLeft;
    }

    SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
    if (x1 > fClipRect.fRight) {
        x1 = fClipRect.fRight;
        SkAlphaRuns::BreakAt((int16_t*)runs, (uint8_t*)aa, x1 - x0);
        ((int16_t*)runs)[x1 - x0] = 0;
    }

    SkASSERT(x0 < x1 && runs[x1 - x0] == 0);
    SkASSERT(compute_anti_width(runs) == x1 - x0);

    fBlitter->blitAntiH(x0, y, aa, runs);
}

void SkRectClipBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(height > 0);

    if (!x_in_rect(x, fClipRect)) {
        return;
    }

    int y0 = y;
    int y1 = y + height;

    if (y0 < fClipRect.fTop) {
        y0 = fClipRect.fTop;
    }
    if (y1 > fClipRect.fBottom) {
        y1 = fClipRect.fBottom;
    }

    if (y0 < y1) {
        fBlitter->blitV(x, y0, y1 - y0, alpha);
    }
}

void SkRectClipBlitter::blitRect(int left, int y, int width, int height) {
    SkIRect    r;

    r.set(left, y, left + width, y + height);
    if (r.intersect(fClipRect)) {
        fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
    }
}

void SkRectClipBlitter::blitAntiRect(int left, int y, int width, int height,
                                     SkAlpha leftAlpha, SkAlpha rightAlpha) {
    SkIRect    r;

    // The *true* width of the rectangle blitted is width+2:
    r.set(left, y, left + width + 2, y + height);
    if (r.intersect(fClipRect)) {
        if (r.fLeft != left) {
            SkASSERT(r.fLeft > left);
            leftAlpha = 255;
        }
        if (r.fRight != left + width + 2) {
            SkASSERT(r.fRight < left + width + 2);
            rightAlpha = 255;
        }
        if (255 == leftAlpha && 255 == rightAlpha) {
            fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        } else if (1 == r.width()) {
            if (r.fLeft == left) {
                fBlitter->blitV(r.fLeft, r.fTop, r.height(), leftAlpha);
            } else {
                SkASSERT(r.fLeft == left + width + 1);
                fBlitter->blitV(r.fLeft, r.fTop, r.height(), rightAlpha);
            }
        } else {
            fBlitter->blitAntiRect(r.fLeft, r.fTop, r.width() - 2, r.height(),
                                   leftAlpha, rightAlpha);
        }
    }
}

void SkRectClipBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    SkIRect    r = clip;

    if (r.intersect(fClipRect)) {
        fBlitter->blitMask(mask, r);
    }
}

const SkPixmap* SkRectClipBlitter::justAnOpaqueColor(uint32_t* value) {
    return fBlitter->justAnOpaqueColor(value);
}

///////////////////////////////////////////////////////////////////////////////

void SkRgnClipBlitter::blitH(int x, int y, int width) {
    SkRegion::Spanerator span(*fRgn, y, x, x + width);
    int left, right;

    while (span.next(&left, &right)) {
        SkASSERT(left < right);
        fBlitter->blitH(left, y, right - left);
    }
}

void SkRgnClipBlitter::blitAntiH(int x, int y, const SkAlpha aa[],
                                 const int16_t runs[]) {
    int width = compute_anti_width(runs);
    SkRegion::Spanerator span(*fRgn, y, x, x + width);
    int left, right;
    SkDEBUGCODE(const SkIRect& bounds = fRgn->getBounds();)

    int prevRite = x;
    while (span.next(&left, &right)) {
        SkASSERT(x <= left);
        SkASSERT(left < right);
        SkASSERT(left >= bounds.fLeft && right <= bounds.fRight);

        SkAlphaRuns::Break((int16_t*)runs, (uint8_t*)aa, left - x, right - left);

        // now zero before left
        if (left > prevRite) {
            int index = prevRite - x;
            ((uint8_t*)aa)[index] = 0;   // skip runs after right
            ((int16_t*)runs)[index] = SkToS16(left - prevRite);
        }

        prevRite = right;
    }

    if (prevRite > x) {
        ((int16_t*)runs)[prevRite - x] = 0;

        if (x < 0) {
            int skip = runs[0];
            SkASSERT(skip >= -x);
            aa += skip;
            runs += skip;
            x += skip;
        }
        fBlitter->blitAntiH(x, y, aa, runs);
    }
}

void SkRgnClipBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkIRect    bounds;
    bounds.set(x, y, x + 1, y + height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));

        fBlitter->blitV(x, r.fTop, r.height(), alpha);
        iter.next();
    }
}

void SkRgnClipBlitter::blitRect(int x, int y, int width, int height) {
    SkIRect    bounds;
    bounds.set(x, y, x + width, y + height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));

        fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        iter.next();
    }
}

void SkRgnClipBlitter::blitAntiRect(int x, int y, int width, int height,
                                    SkAlpha leftAlpha, SkAlpha rightAlpha) {
    // The *true* width of the rectangle to blit is width + 2
    SkIRect    bounds;
    bounds.set(x, y, x + width + 2, y + height);

    SkRegion::Cliperator    iter(*fRgn, bounds);

    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));
        SkASSERT(r.fLeft >= x);
        SkASSERT(r.fRight <= x + width + 2);

        SkAlpha effectiveLeftAlpha = (r.fLeft == x) ? leftAlpha : 255;
        SkAlpha effectiveRightAlpha = (r.fRight == x + width + 2) ?
                                      rightAlpha : 255;

        if (255 == effectiveLeftAlpha && 255 == effectiveRightAlpha) {
            fBlitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
        } else if (1 == r.width()) {
            if (r.fLeft == x) {
                fBlitter->blitV(r.fLeft, r.fTop, r.height(),
                                effectiveLeftAlpha);
            } else {
                SkASSERT(r.fLeft == x + width + 1);
                fBlitter->blitV(r.fLeft, r.fTop, r.height(),
                                effectiveRightAlpha);
            }
        } else {
            fBlitter->blitAntiRect(r.fLeft, r.fTop, r.width() - 2, r.height(),
                                   effectiveLeftAlpha, effectiveRightAlpha);
        }
        iter.next();
    }
}


void SkRgnClipBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    SkRegion::Cliperator iter(*fRgn, clip);
    const SkIRect&       r = iter.rect();
    SkBlitter*           blitter = fBlitter;

    while (!iter.done()) {
        blitter->blitMask(mask, r);
        iter.next();
    }
}

const SkPixmap* SkRgnClipBlitter::justAnOpaqueColor(uint32_t* value) {
    return fBlitter->justAnOpaqueColor(value);
}

///////////////////////////////////////////////////////////////////////////////

SkBlitter* SkBlitterClipper::apply(SkBlitter* blitter, const SkRegion* clip,
                                   const SkIRect* ir) {
    if (clip) {
        const SkIRect& clipR = clip->getBounds();

        if (clip->isEmpty() || (ir && !SkIRect::Intersects(clipR, *ir))) {
            blitter = &fNullBlitter;
        } else if (clip->isRect()) {
            if (ir == nullptr || !clipR.contains(*ir)) {
                fRectBlitter.init(blitter, clipR);
                blitter = &fRectBlitter;
            }
        } else {
            fRgnBlitter.init(blitter, clip);
            blitter = &fRgnBlitter;
        }
    }
    return blitter;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkColorShader.h"
#include "SkColorPriv.h"

class Sk3DShader : public SkShader {
public:
    Sk3DShader(sk_sp<SkShader> proxy) : fProxy(std::move(proxy)) {}

    Context* onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const override {
        SkShader::Context* proxyContext = nullptr;
        if (fProxy) {
            proxyContext = fProxy->makeContext(rec, alloc);
            if (!proxyContext) {
                return nullptr;
            }
        }
        return alloc->make<Sk3DShaderContext>(*this, rec, proxyContext);
    }

    class Sk3DShaderContext : public SkShader::Context {
    public:
        // Calls proxyContext's destructor but will NOT free its memory.
        Sk3DShaderContext(const Sk3DShader& shader, const ContextRec& rec,
                          SkShader::Context* proxyContext)
            : INHERITED(shader, rec)
            , fMask(nullptr)
            , fProxyContext(proxyContext)
        {
            if (!fProxyContext) {
                fPMColor = SkPreMultiplyColor(rec.fPaint->getColor());
            }
        }

        ~Sk3DShaderContext() override {
            if (fProxyContext) {
                fProxyContext->~Context();
            }
        }

        void set3DMask(const SkMask* mask) override { fMask = mask; }

        void shadeSpan(int x, int y, SkPMColor span[], int count) override {
            if (fProxyContext) {
                fProxyContext->shadeSpan(x, y, span, count);
            }

            if (fMask == nullptr) {
                if (fProxyContext == nullptr) {
                    sk_memset32(span, fPMColor, count);
                }
                return;
            }

            SkASSERT(fMask->fBounds.contains(x, y));
            SkASSERT(fMask->fBounds.contains(x + count - 1, y));

            size_t          size = fMask->computeImageSize();
            const uint8_t*  alpha = fMask->getAddr8(x, y);
            const uint8_t*  mulp = alpha + size;
            const uint8_t*  addp = mulp + size;

            if (fProxyContext) {
                for (int i = 0; i < count; i++) {
                    if (alpha[i]) {
                        SkPMColor c = span[i];
                        if (c) {
                            unsigned a = SkGetPackedA32(c);
                            unsigned r = SkGetPackedR32(c);
                            unsigned g = SkGetPackedG32(c);
                            unsigned b = SkGetPackedB32(c);

                            unsigned mul = SkAlpha255To256(mulp[i]);
                            unsigned add = addp[i];

                            r = SkFastMin32(SkAlphaMul(r, mul) + add, a);
                            g = SkFastMin32(SkAlphaMul(g, mul) + add, a);
                            b = SkFastMin32(SkAlphaMul(b, mul) + add, a);

                            span[i] = SkPackARGB32(a, r, g, b);
                        }
                    } else {
                        span[i] = 0;
                    }
                }
            } else {    // color
                unsigned a = SkGetPackedA32(fPMColor);
                unsigned r = SkGetPackedR32(fPMColor);
                unsigned g = SkGetPackedG32(fPMColor);
                unsigned b = SkGetPackedB32(fPMColor);
                for (int i = 0; i < count; i++) {
                    if (alpha[i]) {
                        unsigned mul = SkAlpha255To256(mulp[i]);
                        unsigned add = addp[i];

                        span[i] = SkPackARGB32( a,
                                        SkFastMin32(SkAlphaMul(r, mul) + add, a),
                                        SkFastMin32(SkAlphaMul(g, mul) + add, a),
                                        SkFastMin32(SkAlphaMul(b, mul) + add, a));
                    } else {
                        span[i] = 0;
                    }
                }
            }
        }

    private:
        // Unowned.
        const SkMask*       fMask;
        // Memory is unowned, but we need to call the destructor.
        SkShader::Context*  fProxyContext;
        SkPMColor           fPMColor;

        typedef SkShader::Context INHERITED;
    };

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        str->append("Sk3DShader: (");

        if (fProxy) {
            str->append("Proxy: ");
            fProxy->toString(str);
        }

        this->INHERITED::toString(str);

        str->append(")");
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Sk3DShader)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fProxy.get());
    }

private:
    sk_sp<SkShader> fProxy;

    typedef SkShader INHERITED;
};

sk_sp<SkFlattenable> Sk3DShader::CreateProc(SkReadBuffer& buffer) {
    return sk_make_sp<Sk3DShader>(buffer.readShader());
}

class Sk3DBlitter : public SkBlitter {
public:
    Sk3DBlitter(SkBlitter* proxy, SkShader::Context* shaderContext)
        : fProxy(proxy)
        , fShaderContext(shaderContext)
    {}

    void blitH(int x, int y, int width) override {
        fProxy->blitH(x, y, width);
    }

    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override {
        fProxy->blitAntiH(x, y, antialias, runs);
    }

    void blitV(int x, int y, int height, SkAlpha alpha) override {
        fProxy->blitV(x, y, height, alpha);
    }

    void blitRect(int x, int y, int width, int height) override {
        fProxy->blitRect(x, y, width, height);
    }

    void blitMask(const SkMask& mask, const SkIRect& clip) override {
        if (mask.fFormat == SkMask::k3D_Format) {
            fShaderContext->set3DMask(&mask);

            ((SkMask*)&mask)->fFormat = SkMask::kA8_Format;
            fProxy->blitMask(mask, clip);
            ((SkMask*)&mask)->fFormat = SkMask::k3D_Format;

            fShaderContext->set3DMask(nullptr);
        } else {
            fProxy->blitMask(mask, clip);
        }
    }

private:
    // Both pointers are unowned. They will be deleted by SkSmallAllocator.
    SkBlitter*          fProxy;
    SkShader::Context*  fShaderContext;
};

///////////////////////////////////////////////////////////////////////////////

#include "SkCoreBlitters.h"

SkShader::ContextRec::DstType SkBlitter::PreferredShaderDest(const SkImageInfo& dstInfo) {
#ifdef SK_FORCE_PM4f_FOR_L32_BLITS
    return SkShader::ContextRec::kPM4f_DstType;
#else
    return (dstInfo.gammaCloseToSRGB() || dstInfo.colorType() == kRGBA_F16_SkColorType)
            ? SkShader::ContextRec::kPM4f_DstType
            : SkShader::ContextRec::kPMColor_DstType;
#endif
}

bool SkBlitter::UseRasterPipelineBlitter(const SkPixmap& device, const SkPaint& paint) {
#if defined(SK_FORCE_RASTER_PIPELINE_BLITTER)
    return true;
#else
    // By policy we choose not to handle legacy 8888 with SkRasterPipelineBlitter.
    if (device.colorSpace()) {
        return true;
    }

    // ... unless the shader is raster pipeline-only.
    if (paint.getShader() && paint.getShader()->isRasterPipelineOnly()) {
        return true;
    }

    return device.colorType() != kN32_SkColorType;
#endif
}

SkBlitter* SkBlitter::Choose(const SkPixmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& origPaint,
                             SkArenaAlloc* alloc,
                             bool drawCoverage) {
    SkASSERT(alloc != nullptr);

    // which check, in case we're being called by a client with a dummy device
    // (e.g. they have a bounder that always aborts the draw)
    if (kUnknown_SkColorType == device.colorType() ||
            (drawCoverage && (kAlpha_8_SkColorType != device.colorType()))) {
        return alloc->make<SkNullBlitter>();
    }

    SkShader* shader = origPaint.getShader();
    SkColorFilter* cf = origPaint.getColorFilter();
    SkBlendMode mode = origPaint.getBlendMode();
    sk_sp<Sk3DShader> shader3D;

    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    if (origPaint.getMaskFilter() != nullptr &&
            origPaint.getMaskFilter()->getFormat() == SkMask::k3D_Format) {
        shader3D = sk_make_sp<Sk3DShader>(sk_ref_sp(shader));
        // we know we haven't initialized lazyPaint yet, so just do it
        paint.writable()->setShader(shader3D);
        shader = shader3D.get();
    }

    if (mode != SkBlendMode::kSrcOver) {
        bool deviceIsOpaque = kRGB_565_SkColorType == device.colorType();
        switch (SkInterpretXfermode(*paint, deviceIsOpaque)) {
            case kSrcOver_SkXfermodeInterpretation:
                mode = SkBlendMode::kSrcOver;
                paint.writable()->setBlendMode(mode);
                break;
            case kSkipDrawing_SkXfermodeInterpretation:{
                return alloc->make<SkNullBlitter>();
            }
            default:
                break;
        }
    }

    /*
     *  If the xfermode is CLEAR, then we can completely ignore the installed
     *  color/shader/colorfilter, and just pretend we're SRC + color==0. This
     *  will fall into our optimizations for SRC mode.
     */
    if (mode == SkBlendMode::kClear) {
        SkPaint* p = paint.writable();
        p->setShader(nullptr);
        shader = nullptr;
        p->setColorFilter(nullptr);
        cf = nullptr;
        p->setBlendMode(mode = SkBlendMode::kSrc);
        p->setColor(0);
    }

    if (kAlpha_8_SkColorType == device.colorType() && drawCoverage) {
        SkASSERT(nullptr == shader);
        SkASSERT(paint->isSrcOver());
        return alloc->make<SkA8_Coverage_Blitter>(device, *paint);
    }

    if (UseRasterPipelineBlitter(device, *paint)) {
        auto blitter = SkCreateRasterPipelineBlitter(device, *paint, matrix, alloc);
        SkASSERT(blitter);
        return blitter;
    }

    if (nullptr == shader) {
        if (mode != SkBlendMode::kSrcOver) {
            // xfermodes (and filters) require shaders for our current blitters
            paint.writable()->setShader(SkShader::MakeColorShader(paint->getColor()));
            paint.writable()->setAlpha(0xFF);
            shader = paint->getShader();
        } else if (cf) {
            // if no shader && no xfermode, we just apply the colorfilter to
            // our color and move on.
            SkPaint* writablePaint = paint.writable();
            writablePaint->setColor(cf->filterColor(paint->getColor()));
            writablePaint->setColorFilter(nullptr);
            cf = nullptr;
        }
    }

    if (cf) {
        SkASSERT(shader);
        paint.writable()->setShader(shader->makeWithColorFilter(sk_ref_sp(cf)));
        shader = paint->getShader();
        // blitters should ignore the presence/absence of a filter, since
        // if there is one, the shader will take care of it.
    }

    /*
     *  We create a SkShader::Context object, and store it on the blitter.
     */
    SkShader::Context* shaderContext = nullptr;
    if (shader) {
        const SkShader::ContextRec rec(*paint, matrix, nullptr,
                                       PreferredShaderDest(device.info()),
                                       device.colorSpace());
        // Try to create the ShaderContext
        shaderContext = shader->makeContext(rec, alloc);
        if (!shaderContext) {
            return alloc->make<SkNullBlitter>();
        }
        SkASSERT(shaderContext);
    }

    SkBlitter*  blitter = nullptr;
    switch (device.colorType()) {
        case kAlpha_8_SkColorType:
            SkASSERT(!drawCoverage);  // Handled above.
            if (shader) {
                blitter = alloc->make<SkA8_Shader_Blitter>(device, *paint, shaderContext);
            } else {
                blitter = alloc->make<SkA8_Blitter>(device, *paint);
            }
            break;

        case kRGB_565_SkColorType:
            blitter = SkBlitter_ChooseD565(device, *paint, shaderContext, alloc);
            break;

        case kN32_SkColorType:
#ifdef SK_FORCE_PM4f_FOR_L32_BLITS
            if (true)
#else
            if (device.info().gammaCloseToSRGB())
#endif
            {
                blitter = SkBlitter_ARGB32_Create(device, *paint, shaderContext, alloc);
            } else {
                if (shader) {
                        blitter = alloc->make<SkARGB32_Shader_Blitter>(
                                device, *paint, shaderContext);
                } else if (paint->getColor() == SK_ColorBLACK) {
                    blitter = alloc->make<SkARGB32_Black_Blitter>(device, *paint);
                } else if (paint->getAlpha() == 0xFF) {
                    blitter = alloc->make<SkARGB32_Opaque_Blitter>(device, *paint);
                } else {
                    blitter = alloc->make<SkARGB32_Blitter>(device, *paint);
                }
            }
            break;

        case kRGBA_F16_SkColorType:
            blitter = SkBlitter_F16_Create(device, *paint, shaderContext, alloc);
            break;

        default:
            break;
    }

    if (!blitter) {
        blitter = alloc->make<SkNullBlitter>();
    }

    if (shader3D) {
        SkBlitter* innerBlitter = blitter;
        // FIXME - comment about allocator
        // innerBlitter was allocated by allocator, which will delete it.
        // We know shaderContext or its proxies is of type Sk3DShaderContext, so we need to
        // wrapper the blitter to notify it when we see an emboss mask.
        blitter = alloc->make<Sk3DBlitter>(innerBlitter, shaderContext);
    }
    return blitter;
}

///////////////////////////////////////////////////////////////////////////////

class SkZeroShaderContext : public SkShader::Context {
public:
    SkZeroShaderContext(const SkShader& shader, const SkShader::ContextRec& rec)
        // Override rec with the identity matrix, so it is guaranteed to be invertible.
        : INHERITED(shader, SkShader::ContextRec(*rec.fPaint, SkMatrix::I(), nullptr,
                                                 rec.fPreferredDstType, rec.fDstColorSpace)) {}

    void shadeSpan(int x, int y, SkPMColor colors[], int count) override {
        sk_bzero(colors, count * sizeof(SkPMColor));
    }

private:
    typedef SkShader::Context INHERITED;
};

SkShaderBlitter::SkShaderBlitter(const SkPixmap& device, const SkPaint& paint,
                                 SkShader::Context* shaderContext)
        : INHERITED(device)
        , fShader(paint.getShader())
        , fShaderContext(shaderContext) {
    SkASSERT(fShader);
    SkASSERT(fShaderContext);

    fShader->ref();
    fShaderFlags = fShaderContext->getFlags();
    fConstInY = SkToBool(fShaderFlags & SkShader::kConstInY32_Flag);
}

SkShaderBlitter::~SkShaderBlitter() {
    fShader->unref();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkRectClipCheckBlitter::blitH(int x, int y, int width) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, 1)));
    fBlitter->blitH(x, y, width);
}

void SkRectClipCheckBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    const int16_t* iter = runs;
    for (; *iter; iter += *iter)
        ;
    int width = iter - runs;
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, 1)));
    fBlitter->blitAntiH(x, y, aa, runs);
}

void SkRectClipCheckBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 1, height)));
    fBlitter->blitV(x, y, height, alpha);
}

void SkRectClipCheckBlitter::blitRect(int x, int y, int width, int height) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, width, height)));
    fBlitter->blitRect(x, y, width, height);
}

void SkRectClipCheckBlitter::blitAntiRect(int x, int y, int width, int height,
                                     SkAlpha leftAlpha, SkAlpha rightAlpha) {
    bool skipLeft = !leftAlpha;
    bool skipRight = !rightAlpha;
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x + skipLeft, y,
            width + 2 - skipRight - skipLeft, height)));
    fBlitter->blitAntiRect(x, y, width, height, leftAlpha, rightAlpha);
}

void SkRectClipCheckBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));
    SkASSERT(fClipRect.contains(clip));
    fBlitter->blitMask(mask, clip);
}

const SkPixmap* SkRectClipCheckBlitter::justAnOpaqueColor(uint32_t* value) {
    return fBlitter->justAnOpaqueColor(value);
}

void SkRectClipCheckBlitter::blitAntiH2(int x, int y, U8CPU a0, U8CPU a1) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 2, 1)));
    fBlitter->blitAntiH2(x, y, a0, a1);
}

void SkRectClipCheckBlitter::blitAntiV2(int x, int y, U8CPU a0, U8CPU a1) {
    SkASSERT(fClipRect.contains(SkIRect::MakeXYWH(x, y, 1, 2)));
    fBlitter->blitAntiV2(x, y, a0, a1);
}

#endif
