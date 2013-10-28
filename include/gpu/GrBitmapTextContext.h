/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBitmapTextContext_DEFINED
#define GrBitmapTextContext_DEFINED

#include "GrTextContext.h"

class GrTextStrike;

/*
 * This class implements GrTextContext using standard bitmap fonts
 */
class GrBitmapTextContext : public GrTextContext {
public:
    GrBitmapTextContext(GrContext*, const GrPaint&, SkColor);
    virtual ~GrBitmapTextContext();

    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                         GrFontScaler*) SK_OVERRIDE;

private:
    GrContext::AutoMatrix  fAutoMatrix;
    GrTextStrike*          fStrike;

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
