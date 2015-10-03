/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilAndCoverTextContext_DEFINED
#define GrStencilAndCoverTextContext_DEFINED

#include "GrTextContext.h"
#include "GrDrawTarget.h"
#include "GrStrokeInfo.h"

class GrTextStrike;
class GrPath;
class GrPathRange;
class GrPathRangeDraw;
class SkSurfaceProps;

/*
 * This class implements text rendering using stencil and cover path rendering
 * (by the means of GrDrawTarget::drawPath).
 * This class exposes the functionality through GrTextContext interface.
 */
class GrStencilAndCoverTextContext : public GrTextContext {
public:
    static GrStencilAndCoverTextContext* Create(GrContext*, const SkSurfaceProps&);

    virtual ~GrStencilAndCoverTextContext();

private:
    SkScalar                                            fTextRatio;
    float                                               fTextInverseRatio;
    SkGlyphCache*                                       fGlyphCache;

    GrPathRange*                                        fGlyphs;
    GrPathRangeDraw*                                    fDraw;
    GrStrokeInfo                                        fStroke;
    SkSTArray<32, uint16_t, true>                       fFallbackIndices;
    SkSTArray<32, SkPoint, true>                        fFallbackPositions;

    SkMatrix                                            fViewMatrix;
    SkMatrix                                            fLocalMatrix;
    SkAutoTUnref<GrRenderTarget>                        fRenderTarget;
    GrClip                                              fClip;
    SkIRect                                             fClipRect;
    SkIRect                                             fRegionClipBounds;
    GrPaint                                             fPaint;
    SkPaint                                             fSkPaint;

    GrStencilAndCoverTextContext(GrContext*, const SkSurfaceProps&);

    bool canDraw(const GrRenderTarget*, const GrClip&, const GrPaint&,
                 const SkPaint&, const SkMatrix& viewMatrix) override;

    void onDrawText(GrDrawContext*, GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                    const SkMatrix& viewMatrix,
                    const char text[], size_t byteLength,
                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) override;
    void onDrawPosText(GrDrawContext*, GrRenderTarget*,
                       const GrClip&, const GrPaint&, const SkPaint&,
                       const SkMatrix& viewMatrix,
                       const char text[], size_t byteLength,
                       const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset, const SkIRect& regionClipBounds) override;

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&, size_t textByteLength,
              const SkMatrix& viewMatrix, const SkIRect& regionClipBounds);
    void appendGlyph(const SkGlyph&, const SkPoint&);
    void flush(GrDrawContext* dc);
    void finish(GrDrawContext* dc);

    typedef GrTextContext INHERITED;
};

#endif
