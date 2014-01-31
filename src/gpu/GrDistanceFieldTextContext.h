/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldTextContext_DEFINED
#define GrDistanceFieldTextContext_DEFINED

#include "GrTextContext.h"

class GrTextStrike;

/*
 * This class implements GrTextContext using distance field fonts
 */
class GrDistanceFieldTextContext : public GrTextContext {
public:
    GrDistanceFieldTextContext(GrContext*, const SkDeviceProperties&);
    virtual ~GrDistanceFieldTextContext();

    virtual void drawText(const GrPaint&, const SkPaint&, const char text[], size_t byteLength,
                          SkScalar x, SkScalar y) SK_OVERRIDE;
    virtual void drawPosText(const GrPaint&, const SkPaint&,
                             const char text[], size_t byteLength,
                             const SkScalar pos[], SkScalar constY,
                             int scalarsPerPosition) SK_OVERRIDE;

    virtual bool canDraw(const SkPaint& paint) SK_OVERRIDE;

private:
    GrTextStrike*           fStrike;
    SkScalar                fTextRatio;

    void init(const GrPaint&, const SkPaint&);
    void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top, GrFontScaler*);
    void flushGlyphs();                 // automatically called by destructor
    void finish();

    enum {
        kMinRequestedGlyphs      = 1,
        kDefaultRequestedGlyphs  = 64,
        kMinRequestedVerts       = kMinRequestedGlyphs * 4,
        kDefaultRequestedVerts   = kDefaultRequestedGlyphs * 4,
    };

    SkPoint*                fVertices;
    int32_t                 fMaxVertices;
    GrTexture*              fCurrTexture;
    int                     fCurrVertex;
};

#endif
