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
    virtual void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                         GrFontScaler*) SK_OVERRIDE;

private:
    GrBitmapTextContext(GrContext*, const GrPaint&, const SkPaint&);
    virtual ~GrBitmapTextContext();
    friend class GrTTextContextManager<GrBitmapTextContext>;

    GrContext::AutoMatrix  fAutoMatrix;
    GrTextStrike*          fStrike;

    void flushGlyphs();                 // automatically called by destructor

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
