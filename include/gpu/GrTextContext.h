
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrGlyph.h"
#include "GrPaint.h"
#include "GrMatrix.h"

struct GrGpuTextVertex;
class GrContext;
class GrTextStrike;
class GrFontScaler;
class GrDrawTarget;

class GrTextContext {
public:
    GrTextContext(GrContext*,
                  const GrPaint& paint,
                  const GrMatrix* extMatrix = NULL);
    ~GrTextContext();

    void drawPackedGlyph(GrGlyph::PackedID, GrFixed left, GrFixed top,
                         GrFontScaler*);

    void flush();   // optional; automatically called by destructor

private:
    GrPaint         fPaint;
    GrVertexLayout  fVertexLayout;
    GrContext*      fContext;
    GrDrawTarget*   fDrawTarget;

    GrMatrix        fExtMatrix;
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

    GrGpuTextVertex* fVertices;

    int32_t     fMaxVertices;
    GrTexture*  fCurrTexture;
    int         fCurrVertex;

    GrIRect     fClipRect;
    GrMatrix    fOrigViewMatrix;    // restore previous viewmatrix
};

#endif

