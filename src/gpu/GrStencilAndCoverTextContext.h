/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilAndCoverTextContext_DEFINED
#define GrStencilAndCoverTextContext_DEFINED

#include "GrTextContext.h"
#include "GrDrawState.h"
#include "GrDrawTarget.h"
#include "SkStrokeRec.h"

class GrTextStrike;
class GrPath;
class GrPathRange;

/*
 * This class implements text rendering using stencil and cover path rendering
 * (by the means of GrDrawTarget::drawPath).
 * This class exposes the functionality through GrTextContext interface.
 */
class GrStencilAndCoverTextContext : public GrTextContext {
public:
    GrStencilAndCoverTextContext(GrContext*, const SkDeviceProperties&);
    virtual ~GrStencilAndCoverTextContext();

    virtual void drawText(const GrPaint&, const SkPaint&, const char text[],
                          size_t byteLength,
                          SkScalar x, SkScalar y) SK_OVERRIDE;
    virtual void drawPosText(const GrPaint&, const SkPaint&,
                             const char text[], size_t byteLength,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPosition) SK_OVERRIDE;

    virtual bool canDraw(const SkPaint& paint) SK_OVERRIDE;

private:
    static const int kGlyphBufferSize = 1024;

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

    void init(const GrPaint&, const SkPaint&, size_t textByteLength,
              RenderMode, SkScalar textTranslateY = 0);
    void initGlyphs(SkGlyphCache* cache);
    void appendGlyph(uint16_t glyphID, float x);
    void appendGlyph(uint16_t glyphID, float x, float y);
    void flush();
    void finish();

    GrDrawState::AutoRestoreEffects fStateRestore;
    SkScalar fTextRatio;
    float fTextInverseRatio;
    SkGlyphCache* fGlyphCache;
    GrPathRange* fGlyphs;
    uint32_t fIndexBuffer[kGlyphBufferSize];
    float fTransformBuffer[2 * kGlyphBufferSize];
    GrDrawTarget::PathTransformType fTransformType;
    int fPendingGlyphCount;
    SkMatrix fContextInitialMatrix;
    bool fNeedsDeviceSpaceGlyphs;
};

#endif
