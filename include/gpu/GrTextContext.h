/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrContext.h"
#include "GrGlyph.h"
#include "GrPaint.h"

class GrContext;
class GrTextStrike;
class GrFontScaler;
class GrDrawTarget;

class GrTextContext {
public:
    GrTextContext(GrContext*, const GrPaint&);
    ~GrTextContext();

    void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                         GrFontScaler*);

    void flush();   // optional; automatically called by destructor

private:
    GrPaint         fPaint;
    GrContext*      fContext;
    GrDrawTarget*   fDrawTarget;

    GrFontScaler*   fScaler;
    GrTextStrike*   fStrike;

    inline void flushGlyphs();
    void setupDrawTarget();

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

    SkIRect                 fClipRect;
    GrContext::AutoMatrix   fAutoMatrix;
};

#endif
