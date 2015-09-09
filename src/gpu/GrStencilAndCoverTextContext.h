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
    enum RenderMode {
        /**
         * This is the render mode used by drawText(), which is mainly used by
         * the Skia unit tests. It tries match the other text backends exactly,
         * with the exception of not implementing LCD text, and doing anti-
         * aliasing with the built-in MSAA.
         */
        kMaxAccuracy_RenderMode,

        /**
         * This is the render mode used by drawPosText(). It ignores hinting and
         * LCD text, even if the client provided positions for hinted glyphs,
         * and renders from a canonically-sized, generic set of paths for the
         * given typeface. In the future we should work out a system for the
         * client to know it should not provide hinted glyph positions. This
         * render mode also tries to use GPU stroking for fake bold, even when
         * SK_USE_FREETYPE_EMBOLDEN is set.
         */
        kMaxPerformance_RenderMode,
    };

    SkScalar                                            fTextRatio;
    float                                               fTextInverseRatio;
    SkGlyphCache*                                       fGlyphCache;

    GrPathRange*                                        fGlyphs;
    GrPathRangeDraw*                                    fDraw;
    GrStrokeInfo                                        fStroke;
    SkSTArray<32, uint16_t, true>                       fFallbackIndices;
    SkSTArray<32, SkPoint, true>                        fFallbackPositions;

    SkMatrix                                            fContextInitialMatrix;
    SkMatrix                                            fViewMatrix;
    SkMatrix                                            fLocalMatrix;
    bool                                                fUsingDeviceSpaceGlyphs;
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

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
              size_t textByteLength, RenderMode, const SkMatrix& viewMatrix,
              const SkIRect& regionClipBounds);
    bool mapToFallbackContext(SkMatrix* inverse);
    void appendGlyph(const SkGlyph&, const SkPoint&);
    void flush(GrDrawContext* dc);
    void finish(GrDrawContext* dc);

    typedef GrTextContext INHERITED;
};

#endif
