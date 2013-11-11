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
    GrDistanceFieldTextContext(GrContext*, const GrPaint&, SkColor, SkScalar textRatio);
    virtual ~GrDistanceFieldTextContext();

    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                                 GrFontScaler*) SK_OVERRIDE;

private:
    GrTextStrike*           fStrike;
    SkScalar                fTextRatio;

    void flushGlyphs();                 // automatically called by destructor

    enum {
        kMinRequestedGlyphs      = 1,
        kDefaultRequestedGlyphs  = 64,
        kMinRequestedVerts       = kMinRequestedGlyphs * 4,
        kDefaultRequestedVerts   = kDefaultRequestedGlyphs * 4,
    };

    SkColor                 fSkPaintColor;
    SkPoint*                fVertices;
    int32_t                 fMaxVertices;
    GrTexture*              fCurrTexture;
    int                     fCurrVertex;
};

#endif
