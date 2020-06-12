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
    class Options {
    public:
        Options(SkScalar min, SkScalar max)
                : fMinDistanceFieldFontSize{min}
                , fMaxDistanceFieldFontSize{max} {
            SkASSERT_RELEASE(min > 0 && max >= min);
        }

        bool canDrawAsDistanceFields(const SkPaint&, const SkFont&, const SkMatrix& viewMatrix,
                                     const SkSurfaceProps& props,
                                     bool contextSupportsDistanceFieldText) const;
        SkFont getSDFFont(const SkFont& font,
                          const SkMatrix& viewMatrix,
                          SkScalar* textRatio) const;
        std::pair<SkScalar, SkScalar> computeSDFMinMaxScale(
                SkScalar textSize, const SkMatrix& viewMatrix) const;
    private:
        // Below this size (in device space) distance field text will not be used.
        const SkScalar fMinDistanceFieldFontSize;

        // Above this size (in device space) distance field text will not be used and glyphs will
        // be rendered from outline as individual paths.
        const SkScalar fMaxDistanceFieldFontSize;
    };

    static std::unique_ptr<GrTextContext> Make(const Options& options);

    static SkPaint InitDistanceFieldPaint(const SkPaint& paint);

private:
    GrTextContext(const Options& options);

    Options fOptions;
};

#endif  // GrTextContext_DEFINED
