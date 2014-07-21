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

    void init(const GrPaint&, const SkPaint&, size_t textByteLength);
    void initGlyphs(SkGlyphCache* cache);
    void appendGlyph(uint16_t glyphID, float x, float y);
    void flush();
    void finish();

    GrDrawState::AutoRestoreEffects fStateRestore;
    SkScalar fTextRatio;
    SkStrokeRec fStroke;
    SkGlyphCache* fGlyphCache;
    GlyphPathRange* fGlyphs;
    uint32_t fIndexBuffer[kGlyphBufferSize];
    float fTransformBuffer[6 * kGlyphBufferSize];
    int fPendingGlyphCount;
    SkMatrix fGlyphTransform;
    bool fNeedsDeviceSpaceGlyphs;
};

#endif
