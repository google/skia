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
    void init(const GrPaint&, const SkPaint&, size_t textByteLength);
    void appendGlyph(GrGlyph::PackedID, const SkPoint&,
                     GrTextStrike*, GrFontScaler*);
    void finish();

    GrDrawState::AutoRestoreEffects fStateRestore;
    SkScalar fTextRatio;
    SkStrokeRec fStroke;
    SkTDArray<const GrPath*> fPaths;
    SkTDArray<SkMatrix> fTransforms;
    SkPath fTmpPath;
};

#endif
