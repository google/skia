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
class GrTextBlobCache;
class SkGlyph;

/*
 * Renders text using some kind of an atlas, ie BitmapText or DistanceField text
 */
class GrAtlasTextContext {
public:
    struct Options {
        /**
         * Below this size (in device space) distance field text will not be used. Negative means
         * use a default value.
         */
        SkScalar fMinDistanceFieldFontSize = -1.f;
        /**
         * Above this size (in device space) distance field text will not be used and glyphs will
         * be rendered from outline as individual paths. Negative means use a default value.
         */
        SkScalar fMaxDistanceFieldFontSize = -1.f;
        /** Forces all distance field vertices to use 3 components, not just when in perspective. */
        bool fDistanceFieldVerticesAlwaysHaveW = false;
    };

    static std::unique_ptr<GrAtlasTextContext> Make(const Options& options);

    void drawText(GrContext*, GrTextUtils::Target*, const GrClip&, const SkPaint&,
                  const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                  size_t byteLength, SkScalar x, SkScalar y, const SkIRect& regionClipBounds);
    void drawPosText(GrContext*, GrTextUtils::Target*, const GrClip&, const SkPaint&,
                     const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                     size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& regionClipBounds);
    void drawTextBlob(GrContext*, GrTextUtils::Target*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkTextBlob*,
                      SkScalar x, SkScalar y, SkDrawFilter*, const SkIRect& clipBounds);

private:
    GrAtlasTextContext(const Options& options);

    class FallbackTextHelper {
    public:
        FallbackTextHelper(const SkMatrix& viewMatrix,
                           const SkPaint& pathPaint,
                           const GrGlyphCache* glyphCache,
                           SkScalar textRatio)
            : fViewMatrix(viewMatrix)
            , fTextSize(pathPaint.getTextSize())
            , fMaxTextSize(glyphCache->getGlyphSizeLimit())
            , fTextRatio(textRatio)
            , fScaledFallbackTextSize(fMaxTextSize)
            , fUseScaledFallback(false) {
            fMaxScale = viewMatrix.getMaxScale();
        }

        void appendText(const SkGlyph& glyph, int count, const char* text, SkPoint glyphPos);
        void drawText(GrAtlasTextBlob* blob, int runIndex, GrGlyphCache*, const SkSurfaceProps&,
                      const GrTextUtils::Paint&, SkScalerContextFlags);

    private:
        SkTDArray<char> fFallbackTxt;
        SkTDArray<SkPoint> fFallbackPos;

        const SkMatrix& fViewMatrix;
        SkScalar fTextSize;
        SkScalar fMaxTextSize;
        SkScalar fTextRatio;
        SkScalar fScaledFallbackTextSize;
        SkScalar fMaxScale;
        bool fUseScaledFallback;
    };

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    static SkColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    // Determines if we need to use fake gamma (and contrast boost):
    static SkScalerContextFlags ComputeScalerContextFlags(const GrColorSpaceInfo&);
    void regenerateTextBlob(GrAtlasTextBlob* bmp,
                            GrGlyphCache*,
                            const GrShaderCaps&,
                            const GrTextUtils::Paint&,
                            SkScalerContextFlags scalerContextFlags,
                            const SkMatrix& viewMatrix,
                            const SkSurfaceProps&,
                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                            SkDrawFilter* drawFilter) const;

    static bool HasLCD(const SkTextBlob*);

    sk_sp<GrAtlasTextBlob> makeDrawTextBlob(GrTextBlobCache*, GrGlyphCache*,
                                            const GrShaderCaps&,
                                            const GrTextUtils::Paint&,
                                            SkScalerContextFlags scalerContextFlags,
                                            const SkMatrix& viewMatrix,
                                            const SkSurfaceProps&,
                                            const char text[], size_t byteLength,
                                            SkScalar x, SkScalar y) const;

    sk_sp<GrAtlasTextBlob> makeDrawPosTextBlob(GrTextBlobCache*, GrGlyphCache*,
                                               const GrShaderCaps&,
                                               const GrTextUtils::Paint&,
                                               SkScalerContextFlags scalerContextFlags,
                                               const SkMatrix& viewMatrix,
                                               const SkSurfaceProps&,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[],
                                               int scalarsPerPosition,
                                               const SkPoint& offset) const;

    // Functions for appending BMP text to GrAtlasTextBlob
    static void DrawBmpText(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                            const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                            SkScalerContextFlags scalerContextFlags, const SkMatrix& viewMatrix,
                            const char text[], size_t byteLength, SkScalar x, SkScalar y);

    static void DrawBmpPosText(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                               const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                               SkScalerContextFlags scalerContextFlags, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength, const SkScalar pos[],
                               int scalarsPerPosition, const SkPoint& offset);

    static void DrawBmpTextAsPaths(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                                   const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                                   SkScalerContextFlags scalerContextFlags,
                                   const SkMatrix& viewMatrix, const char text[],
                                   size_t byteLength, SkScalar x, SkScalar y);

    static void DrawBmpPosTextAsPaths(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                                      const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                                      SkScalerContextFlags scalerContextFlags,
                                      const SkMatrix& viewMatrix,
                                      const char text[], size_t byteLength,
                                      const SkScalar pos[], int scalarsPerPosition,
                                      const SkPoint& offset);

    // functions for appending distance field text
    bool canDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                 const SkSurfaceProps& props, const GrShaderCaps& caps) const;

    void drawDFText(GrAtlasTextBlob* blob, int runIndex, GrGlyphCache*, const SkSurfaceProps&,
                    const GrTextUtils::Paint& paint, SkScalerContextFlags scalerContextFlags,
                    const SkMatrix& viewMatrix, const char text[], size_t byteLength, SkScalar x,
                    SkScalar y) const;

    void drawDFPosText(GrAtlasTextBlob* blob, int runIndex, GrGlyphCache*,
                       const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                       SkScalerContextFlags scalerContextFlags,
                       const SkMatrix& viewMatrix, const char text[],
                       size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset) const;

    void initDistanceFieldPaint(GrAtlasTextBlob* blob,
                                SkPaint* skPaint,
                                SkScalar* textRatio,
                                const SkMatrix& viewMatrix) const;

    static void BmpAppendGlyph(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                               sk_sp<GrTextStrike>*, const SkGlyph&, SkScalar sx, SkScalar sy,
                               GrColor color, SkGlyphCache*, SkScalar textRatio);

    static void DfAppendGlyph(GrAtlasTextBlob*, int runIndex, GrGlyphCache*,
                              sk_sp<GrTextStrike>*, const SkGlyph&, SkScalar sx, SkScalar sy,
                              GrColor color, SkGlyphCache* cache, SkScalar textRatio);

    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable.get(); }

    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

    SkScalar fMinDistanceFieldFontSize;
    SkScalar fMaxDistanceFieldFontSize;
    bool fDistanceFieldVerticesAlwaysHaveW;

#if GR_TEST_UTILS
    static const SkScalerContextFlags kTextBlobOpScalerContextFlags =
            SkScalerContextFlags::kFakeGammaAndBoostContrast;
    GR_DRAW_OP_TEST_FRIEND(GrAtlasTextOp);
#endif
};

#endif
