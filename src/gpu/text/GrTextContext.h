/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "GrTextBlob.h"
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
class GrTextContext {
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

    static std::unique_ptr<GrTextContext> Make(const Options& options);

    void drawPosText(GrContext*, GrTextUtils::Target*, const GrClip&, const SkPaint&,
                     const SkMatrix& viewMatrix, const SkSurfaceProps&, const char text[],
                     size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& regionClipBounds);
    void drawTextBlob(GrContext*, GrTextUtils::Target*, const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkTextBlob*,
                      SkScalar x, SkScalar y, const SkIRect& clipBounds);

    std::unique_ptr<GrDrawOp> createOp_TestingOnly(GrContext*,
                                                   GrTextContext*,
                                                   GrRenderTargetContext*,
                                                   const SkPaint&,
                                                   const SkMatrix& viewMatrix,
                                                   const char* text,
                                                   int x,
                                                   int y);

    static void SanitizeOptions(Options* options);
    static bool CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props,
                                        bool contextSupportsDistanceFieldText,
                                        const Options& options);
    static void InitDistanceFieldPaint(GrTextBlob* blob,
                                       SkPaint* skPaint,
                                       const SkMatrix& viewMatrix,
                                       const Options& options,
                                       SkScalar* textRatio,
                                       SkScalerContextFlags* flags);

    class FallbackTextHelper {
    public:
        FallbackTextHelper(const SkMatrix& viewMatrix,
                           const SkPaint& pathPaint,
                           SkScalar maxTextSize,
                           SkScalar textRatio)
                : fViewMatrix(viewMatrix)
                , fTextSize(pathPaint.getTextSize())
                , fMaxTextSize(maxTextSize)
                , fTextRatio(textRatio)
                , fTransformedFallbackTextSize(fMaxTextSize)
                , fUseTransformedFallback(false) {
            fMaxScale = viewMatrix.getMaxScale();
        }

        void appendText(const SkGlyph& glyph, int count, const char* text, SkPoint glyphPos);
        void drawText(GrTextBlob* blob, int runIndex, GrGlyphCache*, const SkSurfaceProps&,
                      const GrTextUtils::Paint&, SkScalerContextFlags);

        void initializeForDraw(SkPaint* paint, SkScalar* textRatio, SkMatrix* matrix) const;
        const SkTDArray<char>& fallbackText() const { return fFallbackTxt; }

    private:
        SkTDArray<char> fFallbackTxt;
        SkTDArray<SkPoint> fFallbackPos;

        const SkMatrix& fViewMatrix;
        SkScalar fTextSize;
        SkScalar fMaxTextSize;
        SkScalar fTextRatio;
        SkScalar fTransformedFallbackTextSize;
        SkScalar fMaxScale;
        bool fUseTransformedFallback;
    };

private:
    GrTextContext(const Options& options);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    static SkColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    // Determines if we need to use fake gamma (and contrast boost):
    static SkScalerContextFlags ComputeScalerContextFlags(const GrColorSpaceInfo&);
    void regenerateTextBlob(GrTextBlob* bmp,
                            GrGlyphCache*,
                            const GrShaderCaps&,
                            const GrTextUtils::Paint&,
                            SkScalerContextFlags scalerContextFlags,
                            const SkMatrix& viewMatrix,
                            const SkSurfaceProps&,
                            const SkTextBlob* blob, SkScalar x, SkScalar y) const;

    static bool HasLCD(const SkTextBlob*);

    sk_sp<GrTextBlob> makeDrawPosTextBlob(GrTextBlobCache*, GrGlyphCache*,
                                               const GrShaderCaps&,
                                               const GrTextUtils::Paint&,
                                               SkScalerContextFlags scalerContextFlags,
                                               const SkMatrix& viewMatrix,
                                               const SkSurfaceProps&,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[],
                                               int scalarsPerPosition,
                                               const SkPoint& offset) const;

    // Functions for appending BMP text to GrTextBlob
    static void DrawBmpPosText(GrTextBlob*, int runIndex, GrGlyphCache*,
                               const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                               SkScalerContextFlags scalerContextFlags, const SkMatrix& viewMatrix,
                               const char text[], size_t byteLength, const SkScalar pos[],
                               int scalarsPerPosition, const SkPoint& offset);

    static void DrawBmpPosTextAsPaths(GrTextBlob*, int runIndex, GrGlyphCache*,
                                      const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                                      SkScalerContextFlags scalerContextFlags,
                                      const SkMatrix& viewMatrix,
                                      const char text[], size_t byteLength,
                                      const SkScalar pos[], int scalarsPerPosition,
                                      const SkPoint& offset);

    // functions for appending distance field text
    void drawDFPosText(GrTextBlob* blob, int runIndex, GrGlyphCache*,
                       const SkSurfaceProps&, const GrTextUtils::Paint& paint,
                       SkScalerContextFlags scalerContextFlags,
                       const SkMatrix& viewMatrix, const char text[],
                       size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                       const SkPoint& offset) const;

    static void BmpAppendGlyph(GrTextBlob*, int runIndex, GrGlyphCache*,
                               sk_sp<GrTextStrike>*, const SkGlyph&, SkScalar sx, SkScalar sy,
                               GrColor color, SkGlyphCache*, SkScalar textRatio, bool needsXform);

    static void DfAppendGlyph(GrTextBlob*, int runIndex, GrGlyphCache*,
                              sk_sp<GrTextStrike>*, const SkGlyph&, SkScalar sx, SkScalar sy,
                              GrColor color, SkGlyphCache* cache, SkScalar textRatio);

    const GrDistanceFieldAdjustTable* dfAdjustTable() const { return fDistanceAdjustTable.get(); }

    sk_sp<const GrDistanceFieldAdjustTable> fDistanceAdjustTable;

    Options fOptions;

#if GR_TEST_UTILS
    static const SkScalerContextFlags kTextBlobOpScalerContextFlags =
            SkScalerContextFlags::kFakeGammaAndBoostContrast;
    GR_DRAW_OP_TEST_FRIEND(GrAtlasTextOp);
#endif
};

#endif  // GrTextContext_DEFINED
