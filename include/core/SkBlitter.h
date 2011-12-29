
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlitter_DEFINED
#define SkBlitter_DEFINED

#include "SkBitmap.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkMask.h"

/** SkBlitter and its subclasses are responsible for actually writing pixels
    into memory. Besides efficiency, they handle clipping and antialiasing.
*/
class SkBlitter {
public:
    virtual ~SkBlitter();

    /// Blit a horizontal run of one or more pixels.
    virtual void blitH(int x, int y, int width);
    /// Blit a horizontal run of antialiased pixels; runs[] is a *sparse*
    /// zero-terminated run-length encoding of spans of constant alpha values.
    virtual void blitAntiH(int x, int y, const SkAlpha antialias[],
                           const int16_t runs[]);
    /// Blit a vertical run of pixels with a constant alpha value.
    virtual void blitV(int x, int y, int height, SkAlpha alpha);
    /// Blit a solid rectangle one or more pixels wide.
    virtual void blitRect(int x, int y, int width, int height);
    /** Blit a rectangle with one alpha-blended column on the left,
        width (zero or more) opaque pixels, and one alpha-blended column
        on the right.
        The result will always be at least two pixels wide.
    */
    virtual void blitAntiRect(int x, int y, int width, int height,
                              SkAlpha leftAlpha, SkAlpha rightAlpha);
    /// Blit a pattern of pixels defined by a rectangle-clipped mask;
    /// typically used for text.
    virtual void blitMask(const SkMask&, const SkIRect& clip);

    /** If the blitter just sets a single value for each pixel, return the
        bitmap it draws into, and assign value. If not, return NULL and ignore
        the value parameter.
    */
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value);

    ///@name non-virtual helpers
    void blitMaskRegion(const SkMask& mask, const SkRegion& clip);
    void blitRectRegion(const SkIRect& rect, const SkRegion& clip);
    void blitRegion(const SkRegion& clip);
    ///@}

    /** @name Factories
        Return the correct blitter to use given the specified context.
     */
    static SkBlitter* Choose(const SkBitmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& paint) {
        return Choose(device, matrix, paint, NULL, 0);
    }

    static SkBlitter* Choose(const SkBitmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& paint,
                             void* storage, size_t storageSize);

    static SkBlitter* ChooseSprite(const SkBitmap& device,
                                   const SkPaint&,
                                   const SkBitmap& src,
                                   int left, int top,
                                   void* storage, size_t storageSize);
    ///@}

private:
};

/** This blitter silently never draws anything.
*/
class SkNullBlitter : public SkBlitter {
public:
    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha[],
                           const int16_t runs[]) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE;
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE;
};

/** Wraps another (real) blitter, and ensures that the real blitter is only
    called with coordinates that have been clipped by the specified clipRect.
    This means the caller need not perform the clipping ahead of time.
*/
class SkRectClipBlitter : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkIRect& clipRect) {
        SkASSERT(!clipRect.isEmpty());
        fBlitter = blitter;
        fClipRect = clipRect;
    }

    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha[],
                           const int16_t runs[]) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitAntiRect(int x, int y, int width, int height,
                     SkAlpha leftAlpha, SkAlpha rightAlpha) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE;
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE;

private:
    SkBlitter*  fBlitter;
    SkIRect     fClipRect;
};

/** Wraps another (real) blitter, and ensures that the real blitter is only
    called with coordinates that have been clipped by the specified clipRgn.
    This means the caller need not perform the clipping ahead of time.
*/
class SkRgnClipBlitter : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkRegion* clipRgn) {
        SkASSERT(clipRgn && !clipRgn->isEmpty());
        fBlitter = blitter;
        fRgn = clipRgn;
    }

    virtual void blitH(int x, int y, int width) SK_OVERRIDE;
    virtual void blitAntiH(int x, int y, const SkAlpha[],
                           const int16_t runs[]) SK_OVERRIDE;
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE;
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE;
    virtual void blitAntiRect(int x, int y, int width, int height,
                     SkAlpha leftAlpha, SkAlpha rightAlpha) SK_OVERRIDE;
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE;
    virtual const SkBitmap* justAnOpaqueColor(uint32_t* value) SK_OVERRIDE;

private:
    SkBlitter*      fBlitter;
    const SkRegion* fRgn;
};

/** Factory to set up the appropriate most-efficient wrapper blitter
    to apply a clip. Returns a pointer to a member, so lifetime must
    be managed carefully.
*/
class SkBlitterClipper {
public:
    SkBlitter*  apply(SkBlitter* blitter, const SkRegion* clip,
                      const SkIRect* bounds = NULL);

private:
    SkNullBlitter       fNullBlitter;
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
};

#endif
