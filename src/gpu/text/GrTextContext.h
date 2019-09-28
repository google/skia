/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextContext_DEFINED
#define GrTextContext_DEFINED

#include "src/core/SkGlyphRun.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/text/GrDistanceFieldAdjustTable.h"
#include "src/gpu/text/GrTextTarget.h"

#if GR_TEST_UTILS
#include "src/gpu/GrDrawOpTest.h"
#endif

class GrDrawOp;
class GrRecordingContext;
class GrTextBlobCache;
class SkGlyph;
class GrTextBlob;

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

    void drawGlyphRunList(GrRecordingContext*, GrTextTarget*, const GrClip&,
                          const SkMatrix& viewMatrix, const SkSurfaceProps&, const SkGlyphRunList&);

    std::unique_ptr<GrDrawOp> createOp_TestingOnly(GrRecordingContext*,
                                                   GrTextContext*,
                                                   GrRenderTargetContext*,
                                                   const SkPaint&, const SkFont&,
                                                   const SkMatrix& viewMatrix,
                                                   const char* text,
                                                   int x,
                                                   int y);

    static void SanitizeOptions(Options* options);
    static bool CanDrawAsDistanceFields(const SkPaint&, const SkFont&, const SkMatrix& viewMatrix,
                                        const SkSurfaceProps& props,
                                        bool contextSupportsDistanceFieldText,
                                        const Options& options);

    static SkFont InitDistanceFieldFont(const SkFont& font,
                                        const SkMatrix& viewMatrix,
                                        const Options& options,
                                        SkScalar* textRatio);

    static SkPaint InitDistanceFieldPaint(const SkPaint& paint);

    static std::pair<SkScalar, SkScalar> InitDistanceFieldMinMaxScale(SkScalar textSize,
                                                                      const SkMatrix& viewMatrix,
                                                                      const Options& options);

private:
    GrTextContext(const Options& options);

    // sets up the descriptor on the blob and returns a detached cache.  Client must attach
    static SkColor ComputeCanonicalColor(const SkPaint&, bool lcd);
    // Determines if we need to use fake gamma (and contrast boost):
    static SkScalerContextFlags ComputeScalerContextFlags(const GrColorSpaceInfo&);

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
