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
#include "src/gpu/text/GrTextTarget.h"

#if GR_TEST_UTILS
#include "src/gpu/GrDrawOpTest.h"
#endif

class GrDrawOp;
class GrRecordingContext;
class GrRenderTargetContext;
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
    };

    static std::unique_ptr<GrTextContext> Make(const Options& options);

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
    Options options() const { return fOptions; }

private:
    GrTextContext(const Options& options);

    Options fOptions;
};

#endif  // GrTextContext_DEFINED
