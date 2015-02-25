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

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual ~GrTextContext();

    bool drawText(GrRenderTarget* rt, const GrClip&,  const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const char text[], size_t byteLength, SkScalar x,
                  SkScalar y);
    bool drawPosText(GrRenderTarget* rt, const GrClip&, const GrPaint&, const SkPaint&,
                     const SkMatrix& viewMatrix,
                     const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset);

protected:
    GrTextContext*                 fFallbackTextContext;
    GrContext*                     fContext;
    SkDeviceProperties             fDeviceProperties;

    SkAutoTUnref<GrRenderTarget>   fRenderTarget;
    GrClip                         fClip;
    GrDrawTarget*                  fDrawTarget;
    SkIRect                        fClipRect;
    GrPaint                        fPaint;
    SkPaint                        fSkPaint;

    GrTextContext(GrContext*, const SkDeviceProperties&);

    virtual bool canDraw(const SkPaint& paint, const SkMatrix& viewMatrix) = 0;

    virtual void onDrawText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                            const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                            SkScalar x, SkScalar y) = 0;
    virtual void onDrawPosText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                               const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset) = 0;

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&);
    void finish() { fDrawTarget = NULL; }

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    // sets extent in stopVector and returns glyph count
    static int MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                           const char text[], size_t byteLength, SkVector* stopVector);
};

#endif
