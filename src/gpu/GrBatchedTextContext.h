
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrBatchedTextContext_DEFINED
#define GrBatchedTextContext_DEFINED

#include "GrPaint.h"
#include "GrTextContext.h"

class GrDrawTarget;
class GrTexture;

/**
 * Base class for TextContexts that can batch multiple glyphs into single draw.
 *
 * Every glyph is encoded on a single texture.
 * Every glyph is enclosed within a quad (formed by triangle fan) represented
 * by 4 vertices.
 */
class GrBatchedTextContext: public GrTextContext {
public:
    virtual ~GrBatchedTextContext();

protected:
    enum {
        kMinRequestedGlyphs      = 1,
        kDefaultRequestedGlyphs  = 64,
        kMinRequestedVerts       = kMinRequestedGlyphs * 4,
        kDefaultRequestedVerts   = kDefaultRequestedGlyphs * 4,
        kGlyphMaskStage          = GrPaint::kTotalStages,
    };

    GrPaint         fGrPaint;
    GrDrawTarget*   fDrawTarget;

    int32_t     fMaxVertices;
    GrTexture*  fCurrTexture;
    int         fCurrVertex;

    GrBatchedTextContext();
    virtual void init(GrContext* context, const GrPaint&,
                      const GrMatrix* extMatrix) SK_OVERRIDE;
    virtual void finish() SK_OVERRIDE;

    /**
     * Prepare to add another glyph to buffer. The glyph is encoded on the
     * texture provided. Make sure we are using the right texture (or switch
     * to a new texture) and that our buffer is big enough.
     */
    void prepareForGlyph(GrTexture*);

    /**
     * Flush the buffer. Called when switching textures.
     * Must be called in finish() method of all derived classes.
     */
    virtual void flush() = 0;

    /**
     * Set up a buffer to hold vertices of given layout.
     * If NULL != *vertexBuff, don't do anything.
     * Might cause flushing, if draw target suggests it.
     */
    void setupVertexBuff(void** vertexBuff, GrVertexLayout vertexLayout);

    /**
     * Reset after flushing.
     */
    void reset();

private:

    typedef GrTextContext INHERITED;
};

#endif
