/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDistanceFieldTextContext_DEFINED
#define GrDistanceFieldTextContext_DEFINED

#include "GrTextContext.h"

class GrGeometryProcessor;
class GrTextStrike;

/*
 * This class implements GrTextContext using distance field fonts
 */
class GrDistanceFieldTextContext : public GrTextContext {
public:
    static GrDistanceFieldTextContext* Create(GrContext*, const SkDeviceProperties&, bool enable);

    virtual ~GrDistanceFieldTextContext();

private:
    enum {
        kMinRequestedGlyphs      = 1,
        kDefaultRequestedGlyphs  = 64,
        kMinRequestedVerts       = kMinRequestedGlyphs * 4,
        kDefaultRequestedVerts   = kDefaultRequestedGlyphs * 4,
    };

    GrTextStrike*                      fStrike;
    SkScalar                           fTextRatio;
    bool                               fUseLCDText;
    bool                               fEnableDFRendering;
    SkAutoTUnref<GrGeometryProcessor>  fCachedGeometryProcessor;
    // Used to check whether fCachedEffect is still valid.
    uint32_t                           fEffectTextureUniqueID;
    SkColor                            fEffectColor;
    uint32_t                           fEffectFlags;
    GrTexture*                         fGammaTexture;
    void*                              fVertices;
    int                                fCurrVertex;
    int                                fAllocVertexCount;
    int                                fTotalVertexCount;
    GrTexture*                         fCurrTexture;
    SkRect                             fVertexBounds;
    SkMatrix                           fViewMatrix;

    GrDistanceFieldTextContext(GrContext*, const SkDeviceProperties&, bool enable);

    bool canDraw(const SkPaint& paint, const SkMatrix& viewMatrix) SK_OVERRIDE;

    virtual void onDrawText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                            const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength,
                            SkScalar x, SkScalar y) SK_OVERRIDE;
    virtual void onDrawPosText(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&,
                               const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength,
                               const SkScalar pos[], int scalarsPerPosition,
                               const SkPoint& offset) SK_OVERRIDE;

    void init(GrRenderTarget*, const GrClip&, const GrPaint&, const SkPaint&);
    bool appendGlyph(GrGlyph::PackedID, SkScalar left, SkScalar top, GrFontScaler*);
    bool uploadGlyph(GrGlyph*, GrFontScaler*);
    void setupCoverageEffect(const SkColor& filteredColor);
    void flush();                 // automatically called by destructor
    void finish();
};

#endif
