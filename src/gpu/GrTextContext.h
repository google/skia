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
#include "SkSurfaceProps.h"
#include "SkPostConfig.h"

class GrClip;
class GrContext;
class GrDrawContext;
class GrFontScaler;
class SkDrawFilter;
class SkTextBlob;

/*
 * This class wraps the state for a single text render
 */
class GrTextContext {
public:
    virtual ~GrTextContext();

    void drawText(GrDrawContext* dc, GrRenderTarget* rt,
                  const GrClip&,  const GrPaint&, const SkPaint&,
                  const SkMatrix& viewMatrix, const char text[], size_t byteLength, SkScalar x,
                  SkScalar y, const SkIRect& clipBounds);
    void drawPosText(GrDrawContext* dc, GrRenderTarget* rt,
                     const GrClip&, const GrPaint&, const SkPaint&,
                     const SkMatrix& viewMatrix,
                     const char text[], size_t byteLength,
                     const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds);
    virtual void drawTextBlob(GrDrawContext* dc, GrRenderTarget*, const GrClip&, const SkPaint&,
                              const SkMatrix& viewMatrix, const SkTextBlob*,
                              SkScalar x, SkScalar y,
                              SkDrawFilter*, const SkIRect& clipBounds);

    static bool ShouldDisableLCD(const SkPaint& paint);

protected:
    GrTextContext*                 fFallbackTextContext;
    GrContext*                     fContext;
    SkSurfaceProps                 fSurfaceProps;

    GrTextContext(GrContext*, const SkSurfaceProps&);

    virtual bool canDraw(const SkPaint&, const SkMatrix& viewMatrix) = 0;

    virtual void onDrawText(GrDrawContext*, GrRenderTarget*, const GrClip&,
                            const GrPaint&, const SkPaint&,
                            const SkMatrix& viewMatrix, const char text[], size_t byteLength,
                            SkScalar x, SkScalar y, const SkIRect& clipBounds) = 0;
    virtual void onDrawPosText(GrDrawContext*, GrRenderTarget*, const GrClip&,
                               const GrPaint&, const SkPaint&,
                               const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset, const SkIRect& clipBounds) = 0;

    void drawTextAsPath(GrDrawContext*, const GrClip& clip,
                        const SkPaint& origPaint, const SkMatrix& viewMatrix,
                        const char text[], size_t byteLength, SkScalar x, SkScalar y,
                        const SkIRect& clipBounds);
    void drawPosTextAsPath(GrDrawContext*, const GrClip& clip,
                           const SkPaint& origPaint, const SkMatrix& viewMatrix,
                           const char text[], size_t byteLength,
                           const SkScalar pos[], int scalarsPerPosition,
                           const SkPoint& offset, const SkIRect& clipBounds);

    static GrFontScaler* GetGrFontScaler(SkGlyphCache* cache);
    // sets extent in stopVector and returns glyph count
    static int MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                           const char text[], size_t byteLength, SkVector* stopVector);
    static uint32_t FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint);

    friend class GrAtlasTextBatch;
};

#endif
