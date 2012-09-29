
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBlitter.h"
#include "SkAntiRun.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkFilterShader.h"
#include "SkFlattenableBuffers.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkTemplatesPriv.h"
#include "SkTLazy.h"
#include "SkUtils.h"
#include "SkXfermode.h"

SkBlitter::~SkBlitter() {}

const SkBitmap* SkBlitter::justAnOpaqueColor(uint32_t* value) {
    return NULL;
}

void SkBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("unimplemented");
}

void SkBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                          const int16_t runs[]) {
    SkDEBUGFAIL("unimplemented");
}

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

/// Default implementation doesn't check for any easy optimizations
/// such as alpha == 0 or 255; also uses blitV(), which some subclasses
/// may not support.
void SkBlitter::blitAntiRect(int x, int y, int width, int height,
                             SkAlpha leftAlpha, SkAlpha rightAlpha) {
    this->blitV(x++, y, height, leftAlpha);
    if (width > 0) {
        this->blitRect(x, y, width, height);
        x += width;
    }
    this->blitV(x, y, height, rightAlpha);
}

//////////////////////////////////////////////////////////////////////////////

static inline void bits_to_runs(SkBlitter* blitter, int x, int y,
                                const uint8_t bits[],
                                U8CPU left_mask, int rowBytes,
                                U8CPU right_mask) {
    int inFill = 0;
    int pos = 0;

    while (--rowBytes >= 0) {
        unsigned b = *bits++ & left_mask;
        if (rowBytes == 0) {
            b &= right_mask;
        }

        for (unsigned test = 0x80; test != 0; test >>= 1) {
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
        left_mask = 0xFF;
    }

    // final cleanup
    if (inFill) {
        blitter->blitH(pos, y, x - pos);
    }
}

void SkBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkASSERT(mask.fBounds.contains(clip));

    if (mask.fFormat == SkMask::kBW_Format) {
        int cx = clip.fLeft;
        int cy = clip.fTop;
        int maskLeft = mask.fBounds.fLeft;
        int mask_rowBytes = mask.fRowBytes;
        int height = clip.height();

        const uint8_t* bits = mask.getAddr1(cx, cy);

        if (cx == maskLeft && clip.fRight == mask.fBounds.fRight) {
            while (--height >= 0) {
                bits_to_runs(this, cx, cy, bits, 0xFF, mask_rowBytes, 0xFF);
                bits += mask_rowBytes;
                cy += 1;
            }
        } else {
            int left_edge = cx - maskLeft;
            SkASSERT(left_edge >= 0);
            int rite_edge = clip.fRight - maskLeft;
            SkASSERT(rite_edge > left_edge);

            int left_mask = 0xFF >> (left_edge & 7);
            int rite_mask = 0xFF << (8 - (rite_edge & 7));
            int full_runs = (rite_edge >> 3) - ((left_edge + 7) >> 3);

            // check for empty right mask, so we don't read off the end (or go slower than we need to)
            if (rite_mask == 0) {
                SkASSERT(full_runs >= 0);
                full_runs -= 1;
                rite_mask = 0xFF;
            }
            if (left_mask == 0xFF) {
                full_runs -= 1;
            }

            // back up manually so we can keep in sync with our byte-aligned src
            // have cx reflect our actual starting x-coord
            cx -= left_edge & 7;

            if (full_runs < 0) {
                SkASSERT((left_mask & rite_mask) != 0);
                while (--height >= 0) {
                    bits_to_runs(this, cx, cy, bits, left_mask, 1, rite_mask);
                    bits += mask_rowBytes;
                    cy += 1;
                }
            } else {
                while (--height >= 0) {
                    bits_to_runs(this, cx, cy, bits, left_mask, full_runs + 2, rite_mask);
                    bits += mask_rowBytes;
                    cy += 1;
                }
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

const SkBitmap* SkNullBlitter::justAnOpaqueColor(uint32_t* value) {
    return NULL;
}

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

const SkBitmap* SkRectClipBlitter::justAnOpaqueColor(uint32_t* value) {
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

const SkBitmap* SkRgnClipBlitter::justAnOpaqueColor(uint32_t* value) {
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
            if (ir == NULL || !clipR.contains(*ir)) {
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
    Sk3DShader(SkShader* proxy) : fProxy(proxy) {
        SkSafeRef(proxy);
        fMask = NULL;
    }

    virtual ~Sk3DShader() {
        SkSafeUnref(fProxy);
    }

    void setMask(const SkMask* mask) { fMask = mask; }

    virtual bool setContext(const SkBitmap& device, const SkPaint& paint,
                            const SkMatrix& matrix) {
        if (fProxy) {
            return fProxy->setContext(device, paint, matrix);
        } else {
            fPMColor = SkPreMultiplyColor(paint.getColor());
            return this->INHERITED::setContext(device, paint, matrix);
        }
    }

    virtual void shadeSpan(int x, int y, SkPMColor span[], int count) {
        if (fProxy) {
            fProxy->shadeSpan(x, y, span, count);
        }

        if (fMask == NULL) {
            if (fProxy == NULL) {
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

        if (fProxy) {
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

    virtual void beginSession() {
        this->INHERITED::beginSession();
        if (fProxy) {
            fProxy->beginSession();
        }
    }

    virtual void endSession() {
        if (fProxy) {
            fProxy->endSession();
        }
        this->INHERITED::endSession();
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Sk3DShader)

protected:
    Sk3DShader(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fProxy = buffer.readFlattenableT<SkShader>();
        fPMColor = buffer.readColor();
        fMask = NULL;
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.writeFlattenable(fProxy);
        buffer.writeColor(fPMColor);
    }

private:
    SkShader*       fProxy;
    SkPMColor       fPMColor;
    const SkMask*   fMask;

    typedef SkShader INHERITED;
};

class Sk3DBlitter : public SkBlitter {
public:
    Sk3DBlitter(SkBlitter* proxy, Sk3DShader* shader, void (*killProc)(void*))
            : fProxy(proxy), f3DShader(shader), fKillProc(killProc) {
        shader->ref();
    }

    virtual ~Sk3DBlitter() {
        f3DShader->unref();
        fKillProc(fProxy);
    }

    virtual void blitH(int x, int y, int width) {
        fProxy->blitH(x, y, width);
    }

    virtual void blitAntiH(int x, int y, const SkAlpha antialias[],
                           const int16_t runs[]) {
        fProxy->blitAntiH(x, y, antialias, runs);
    }

    virtual void blitV(int x, int y, int height, SkAlpha alpha) {
        fProxy->blitV(x, y, height, alpha);
    }

    virtual void blitRect(int x, int y, int width, int height) {
        fProxy->blitRect(x, y, width, height);
    }

    virtual void blitMask(const SkMask& mask, const SkIRect& clip) {
        if (mask.fFormat == SkMask::k3D_Format) {
            f3DShader->setMask(&mask);

            ((SkMask*)&mask)->fFormat = SkMask::kA8_Format;
            fProxy->blitMask(mask, clip);
            ((SkMask*)&mask)->fFormat = SkMask::k3D_Format;

            f3DShader->setMask(NULL);
        } else {
            fProxy->blitMask(mask, clip);
        }
    }

private:
    SkBlitter*  fProxy;
    Sk3DShader* f3DShader;
    void        (*fKillProc)(void*);
};

///////////////////////////////////////////////////////////////////////////////

#include "SkCoreBlitters.h"

class SkAutoCallProc {
public:
    typedef void (*Proc)(void*);

    SkAutoCallProc(void* obj, Proc proc)
    : fObj(obj), fProc(proc) {}

    ~SkAutoCallProc() {
        if (fObj && fProc) {
            fProc(fObj);
        }
    }

    void* get() const { return fObj; }

    void* detach() {
        void* obj = fObj;
        fObj = NULL;
        return obj;
    }

private:
    void*   fObj;
    Proc    fProc;
};

static void destroy_blitter(void* blitter) {
    ((SkBlitter*)blitter)->~SkBlitter();
}

static void delete_blitter(void* blitter) {
    SkDELETE((SkBlitter*)blitter);
}

static bool just_solid_color(const SkPaint& paint) {
    if (paint.getAlpha() == 0xFF && paint.getColorFilter() == NULL) {
        SkShader* shader = paint.getShader();
        if (NULL == shader ||
            (shader->getFlags() & SkShader::kOpaqueAlpha_Flag)) {
            return true;
        }
    }
    return false;
}

/** By analyzing the paint (with an xfermode), we may decide we can take
    special action. This enum lists our possible actions
 */
enum XferInterp {
    kNormal_XferInterp,         // no special interpretation, draw normally
    kSrcOver_XferInterp,        // draw as if in srcover mode
    kSkipDrawing_XferInterp     // draw nothing
};

static XferInterp interpret_xfermode(const SkPaint& paint, SkXfermode* xfer,
                                     SkBitmap::Config deviceConfig) {
    SkXfermode::Mode  mode;

    if (SkXfermode::AsMode(xfer, &mode)) {
        switch (mode) {
            case SkXfermode::kSrc_Mode:
                if (just_solid_color(paint)) {
                    return kSrcOver_XferInterp;
                }
                break;
            case SkXfermode::kDst_Mode:
                return kSkipDrawing_XferInterp;
            case SkXfermode::kSrcOver_Mode:
                return kSrcOver_XferInterp;
            case SkXfermode::kDstOver_Mode:
                if (SkBitmap::kRGB_565_Config == deviceConfig) {
                    return kSkipDrawing_XferInterp;
                }
                break;
            case SkXfermode::kSrcIn_Mode:
                if (SkBitmap::kRGB_565_Config == deviceConfig &&
                    just_solid_color(paint)) {
                    return kSrcOver_XferInterp;
                }
                break;
            case SkXfermode::kDstIn_Mode:
                if (just_solid_color(paint)) {
                    return kSkipDrawing_XferInterp;
                }
                break;
            default:
                break;
        }
    }
    return kNormal_XferInterp;
}

SkBlitter* SkBlitter::Choose(const SkBitmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& origPaint,
                             void* storage, size_t storageSize) {
    SkASSERT(storageSize == 0 || storage != NULL);

    SkBlitter*  blitter = NULL;

    // which check, in case we're being called by a client with a dummy device
    // (e.g. they have a bounder that always aborts the draw)
    if (SkBitmap::kNo_Config == device.getConfig()) {
        SK_PLACEMENT_NEW(blitter, SkNullBlitter, storage, storageSize);
        return blitter;
    }

    SkShader* shader = origPaint.getShader();
    SkColorFilter* cf = origPaint.getColorFilter();
    SkXfermode* mode = origPaint.getXfermode();
    Sk3DShader* shader3D = NULL;

    SkTLazy<SkPaint> lazyPaint;
    // we promise not to mutate paint unless we know we've reassigned it from
    // lazyPaint
    SkPaint* paint = const_cast<SkPaint*>(&origPaint);

    if (origPaint.getMaskFilter() != NULL &&
            origPaint.getMaskFilter()->getFormat() == SkMask::k3D_Format) {
        shader3D = SkNEW_ARGS(Sk3DShader, (shader));
        // we know we haven't initialized lazyPaint yet, so just do it
        paint = lazyPaint.set(origPaint);
        paint->setShader(shader3D)->unref();
        shader = shader3D;
    }

    if (NULL != mode) {
        switch (interpret_xfermode(*paint, mode, device.config())) {
            case kSrcOver_XferInterp:
                mode = NULL;
                if (!lazyPaint.isValid()) {
                    paint = lazyPaint.set(origPaint);
                }
                paint->setXfermode(NULL);
                break;
            case kSkipDrawing_XferInterp:
                SK_PLACEMENT_NEW(blitter, SkNullBlitter, storage, storageSize);
                return blitter;
            default:
                break;
        }
    }

    if (NULL == shader) {
#ifdef SK_IGNORE_CF_OPTIMIZATION
        if (mode || cf) {
#else
        if (mode) {
#endif
            // xfermodes (and filters) require shaders for our current blitters
            shader = SkNEW(SkColorShader);
            if (!lazyPaint.isValid()) {
                paint = lazyPaint.set(origPaint);
            }
            paint->setShader(shader)->unref();
        } else if (cf) {
            // if no shader && no xfermode, we just apply the colorfilter to
            // our color and move on.
            if (!lazyPaint.isValid()) {
                paint = lazyPaint.set(origPaint);
            }
            paint->setColor(cf->filterColor(paint->getColor()));
            paint->setColorFilter(NULL);
            cf = NULL;
        }
    }

    if (cf) {
        SkASSERT(shader);
        shader = SkNEW_ARGS(SkFilterShader, (shader, cf));
        if (!lazyPaint.isValid()) {
            paint = lazyPaint.set(origPaint);
        }
        paint->setShader(shader)->unref();
        // blitters should ignore the presence/absence of a filter, since
        // if there is one, the shader will take care of it.
    }

    if (shader && !shader->setContext(device, *paint, matrix)) {
        return SkNEW(SkNullBlitter);
    }

    switch (device.getConfig()) {
        case SkBitmap::kA1_Config:
            SK_PLACEMENT_NEW_ARGS(blitter, SkA1_Blitter,
                                  storage, storageSize, (device, *paint));
            break;

        case SkBitmap::kA8_Config:
            if (shader) {
                SK_PLACEMENT_NEW_ARGS(blitter, SkA8_Shader_Blitter,
                                      storage, storageSize, (device, *paint));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, SkA8_Blitter,
                                      storage, storageSize, (device, *paint));
            }
            break;

        case SkBitmap::kARGB_4444_Config:
            blitter = SkBlitter_ChooseD4444(device, *paint, storage, storageSize);
            break;

        case SkBitmap::kRGB_565_Config:
            blitter = SkBlitter_ChooseD565(device, *paint, storage, storageSize);
            break;

        case SkBitmap::kARGB_8888_Config:
            if (shader) {
                SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Shader_Blitter,
                                      storage, storageSize, (device, *paint));
            } else if (paint->getColor() == SK_ColorBLACK) {
                SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Black_Blitter,
                                      storage, storageSize, (device, *paint));
            } else if (paint->getAlpha() == 0xFF) {
                SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Opaque_Blitter,
                                      storage, storageSize, (device, *paint));
            } else {
                SK_PLACEMENT_NEW_ARGS(blitter, SkARGB32_Blitter,
                                      storage, storageSize, (device, *paint));
            }
            break;

        default:
            SkDEBUGFAIL("unsupported device config");
            SK_PLACEMENT_NEW(blitter, SkNullBlitter, storage, storageSize);
            break;
    }

    if (shader3D) {
        void (*proc)(void*) = ((void*)storage == (void*)blitter) ? destroy_blitter : delete_blitter;
        SkAutoCallProc  tmp(blitter, proc);

        blitter = SkNEW_ARGS(Sk3DBlitter, (blitter, shader3D, proc));
        (void)tmp.detach();
    }
    return blitter;
}

///////////////////////////////////////////////////////////////////////////////

const uint16_t gMask_0F0F = 0xF0F;
const uint32_t gMask_00FF00FF = 0xFF00FF;

///////////////////////////////////////////////////////////////////////////////

SkShaderBlitter::SkShaderBlitter(const SkBitmap& device, const SkPaint& paint)
        : INHERITED(device) {
    fShader = paint.getShader();
    SkASSERT(fShader);

    fShader->ref();
    fShader->beginSession();
    fShaderFlags = fShader->getFlags();
}

SkShaderBlitter::~SkShaderBlitter() {
    fShader->endSession();
    fShader->unref();
}

