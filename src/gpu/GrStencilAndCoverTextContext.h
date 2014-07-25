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
    class GlyphPathRange;
    static const int kGlyphBufferSize = 1024;

    enum DeviceSpaceGlyphsBehavior {
        kUseIfNeeded_DeviceSpaceGlyphsBehavior,
        kDoNotUse_DeviceSpaceGlyphsBehavior,
    };
    void init(const GrPaint&, const SkPaint&, size_t textByteLength,
              DeviceSpaceGlyphsBehavior, SkScalar textTranslateY = 0);
    void initGlyphs(SkGlyphCache* cache);
    void appendGlyph(uint16_t glyphID, float x);
    void appendGlyph(uint16_t glyphID, float x, float y);
    void flush();
    void finish();

    GrDrawState::AutoRestoreEffects fStateRestore;
    SkScalar fTextRatio;
    float fTextInverseRatio;
    SkStrokeRec fStroke;
    SkGlyphCache* fGlyphCache;
    GlyphPathRange* fGlyphs;
    uint32_t fIndexBuffer[kGlyphBufferSize];
    float fTransformBuffer[2 * kGlyphBufferSize];
    GrDrawTarget::PathTransformType fTransformType;
    int fPendingGlyphCount;
    SkMatrix fContextInitialMatrix;
    bool fNeedsDeviceSpaceGlyphs;
};

#endif
