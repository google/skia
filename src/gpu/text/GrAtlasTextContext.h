/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasTextContext_DEFINED
#define GrAtlasTextContext_DEFINED

#include "GrAtlasTextBlob.h"
#include "GrDistanceFieldAdjustTable.h"
#include "GrGeometryProcessor.h"
#include "GrTextUtils.h"
#include "SkTextBlobRunIterator.h"

#if GR_TEST_UTILS
#include "GrDrawOpTest.h"
#endif

class GrDrawOp;
class GrRenderTargetContext;
class GrPipelineBuilder;
class GrTextBlobCache;
class SkGlyph;

/*
 * Renders text using some kind of an atlas, ie BitmapText or DistanceField text
 */
class GrAtlasTextContext {
public:
    static GrAtlasTextContext* Create();

    bool canDraw(const SkPaint&, const SkMatrix& viewMatrix, const SkSurfaceProps&,
                 const GrShaderCaps&);

    void drawText(GrContext*, GrRenderTargetContext*, const GrClip&, const SkPaint&,
                  const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                  size_t byteLength, SkScalar x, SkScalar y, const SkIRect& regionClipBounds);
    void drawPosText(GrContext*, GrRenderTargetContext*, const GrClip&, const SkPaint&,
                     const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                     size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& regionClipBounds);
    void drawTextBlob(GrContext*, GrRenderTargetContext*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds);

private:
    GrAtlasTextContext();

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    inline static SkColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    // Determines if we need to use fake gamma (and contrast boost):
    inline static uint32_t ComputeScalerContextFlags(GrRenderTargetContext*);
    static void RegenerateTextBlob(GrAtlasTextBlob* bmp,
                                   GrAtlasGlyphCache*,
                                   const GrShaderCaps&,
                                   const GrTextUtils::Paint&,
                                   uint32_t scalerContextFlags,
                                   const SkMatrix& viewMatrix,
                                   const SkSurfaceProps&,
                                   const SkTextBlob* blob, SkScalar x, SkScalar y,
                                   SkDrawFilter* drawFilter);
    inline static bool HasLCD(const SkTextBlob*);

    static inline sk_sp<GrAtlasTextBlob> MakeDrawTextBlob(GrTextBlobCache*, GrAtlasGlyphCache*,
                                                          const GrShaderCaps&,
                                                          const GrTextUtils::Paint&,
                                                          uint32_t scalerContextFlags,
                                                          const SkMatrix& viewMatrix,
                                                          const SkSurfaceProps&,
                                                          const char text[], size_t byteLength,
                                                          SkScalar x, SkScalar y);
    static inline sk_sp<GrAtlasTextBlob> MakeDrawPosTextBlob(GrTextBlobCache*, GrAtlasGlyphCache*,
                                                             const GrShaderCaps&,
                                                             const GrTextUtils::Paint&,
                                                             uint32_t scalerContextFlags,
                                                             const SkMatrix& viewMatrix,
                                                             const SkSurfaceProps&,
                                                             const char text[], size_t byteLength,
                                                             const SkScalar pos[],
                                                             int scalarsPerPosition,
                                                             const SkPoint& offset);
    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable.get(); }

    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

#if GR_TEST_UTILS
    static const uint32_t kTextBlobOpScalerContextFlags =
            SkPaint::kFakeGammaAndBoostContrast_ScalerContextFlags;
    DRAW_OP_TEST_FRIEND(TextBlobOp);
#endif
};

#endif
