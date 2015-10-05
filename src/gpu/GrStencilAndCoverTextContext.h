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

    class TextRun {
    public:
        TextRun(const SkPaint& fontAndStroke);
        ~TextRun();

        void setText(const char text[], size_t byteLength, SkScalar x, SkScalar y,
                     GrContext*, const SkSurfaceProps*);

        void setPosText(const char text[], size_t byteLength,
                        const SkScalar pos[], int scalarsPerPosition, const SkPoint& offset,
                        GrContext*, const SkSurfaceProps*);

        void draw(GrDrawContext*, GrRenderTarget*, const GrClip&, const GrPaint&, const SkMatrix&,
                  const SkIRect& regionClipBounds, GrTextContext* fallbackTextContext,
                  const SkPaint& originalSkPaint) const;

    private:
        GrPathRange* createGlyphs(GrContext*, SkGlyphCache*);

        void appendGlyph(const SkGlyph&, const SkPoint&);

        GrStrokeInfo                     fStroke;
        SkPaint                          fFont;
        SkScalar                         fTextRatio;
        float                            fTextInverseRatio;
        SkMatrix                         fLocalMatrix;
        bool                             fUsingRawGlyphPaths;
        SkAutoTUnref<GrPathRangeDraw>    fDraw;
        SkSTArray<32, uint16_t, true>    fFallbackIndices;
        SkSTArray<32, SkPoint, true>     fFallbackPositions;
    };

    typedef GrTextContext INHERITED;
};

#endif
