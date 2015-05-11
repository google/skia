/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrClip.h"
#include "GrGlyph.h"
#include "GrPaint.h"
#include "SkDeviceProperties.h"
#include "SkPostConfig.h"

class GrClip;
class GrContext;
class GrDrawTarget;
class GrFontScaler;
class SkDrawFilter;
class SkGpuDevice;
class SkTextBlob;

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual ~GrTextContext();

    void drawText(GrRenderTarget* rt, const GrClip&,  const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const char text[], size_t byteLength, SkScalar x,
                  SkScalar y, const SkIRect& clipBounds);
    void drawPosText(GrRenderTarget* rt, const GrClip&, const GrPaint&, const SkPaint&,
                     const SkMatrix& viewMatrix,
                     const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds);
    virtual void drawTextBlob(GrRenderTarget*, const GrClip&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkTextBlob*, SkScalar x, SkScalar y,
                              SkDrawFilter*, const SkIRect& clipBounds);

protected:
    GrTextContext*                 fFallbackTextContext;
    GrContext*                     fContext;
    // TODO we probably don't really need to store a back pointer to the owning SkGpuDevice, except
    // we need to be able to call drawPath on it in the event no other text context can draw the
    // text.  We might be able to move this logic to context though.  This is unreffed because
    // GrTextContext is completely owned by SkGpuDevice
    SkGpuDevice*                   fGpuDevice;
    SkDeviceProperties             fDeviceProperties;

    SkAutoTUnref<GrRenderTarget>   fRenderTarget;
    GrClip                         fClip;
    GrDrawTarget*                  fDrawTarget;
    SkIRect                        fClipRect;
    SkIRect                        fRegionClipBounds;
    GrPaint                        fPaint;
    SkPaint                        fSkPaint;

    GrTextContext(GrContext*, SkGpuDevice*, const SkDeviceProperties&);

    virtual bool canDraw(const GrRenderTarget*, const GrClip&, const GrPaint&,
                         const SkPaint&, const SkMatrix& viewMatrix) = 0;

    virtual void onDrawText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                            const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                            SkScalar x, SkScalar y, const SkIRect& clipBounds) = 0;
    virtual void onDrawPosText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                               const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset, const SkIRect& clipBounds) = 0;

    void drawTextAsPath(const SkPaint& origPaint, const SkMatrix& viewMatrix,
                        const char text[], size_t byteLength, SkScalar x, SkScalar y,
                        const SkIRect& clipBounds);
    void drawPosTextAsPath(const SkPaint& origPaint, const SkMatrix& viewMatrix,
                           const char text[], size_t byteLength,
                           const SkScalar pos[], int scalarsPerPosition,
                           const SkPoint& offset, const SkIRect& clipBounds);

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
              const SkIRect& regionClipBounds);
    void finish() { fDrawTarget = NULL; }

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    // sets extent in stopVector and returns glyph count
    static int MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                           const char text[], size_t byteLength, SkVector* stopVector);

    friend class BitmapTextBatch;
};

#endif
