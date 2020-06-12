/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrTextContext.h"

#include "include/core/SkGraphics.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTo.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkDraw.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkGlyphRun.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPaintPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/text/GrSDFMaskFilter.h"
#include "src/gpu/text/GrTextBlobCache.h"

// DF sizes and thresholds for usage of the small and medium sizes. For example, above
// kSmallDFFontLimit we will use the medium size. The large size is used up until the size at
// which we switch over to drawing as paths as controlled by Options.
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;
#ifdef SK_BUILD_FOR_MAC
static const int kLargeDFFontLimit = 162;
static const int kExtraLargeDFFontSize = 256;
#endif

GrTextContext::GrTextContext(const Options& options) : fOptions(options) { }

std::unique_ptr<GrTextContext> GrTextContext::Make(const Options& options) {
    return std::unique_ptr<GrTextContext>(new GrTextContext(options));
}

bool GrTextContext::Options::canDrawAsDistanceFields(const SkPaint& paint, const SkFont& font,
                                                     const SkMatrix& viewMatrix,
                                                     const SkSurfaceProps& props,
                                                     bool contextSupportsDistanceFieldText) const {
    // mask filters modify alpha, which doesn't translate well to distance
    if (paint.getMaskFilter() || !contextSupportsDistanceFieldText) {
        return false;
    }

    // TODO: add some stroking support
    if (paint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    if (viewMatrix.hasPerspective()) {
        // Don't use SDF for perspective. Paths look better.
        return false;
    } else {
        SkScalar maxScale = viewMatrix.getMaxScale();
        SkScalar scaledTextSize = maxScale * font.getSize();
        // Hinted text looks far better at small resolutions
        // Scaling up beyond 2x yields undesirable artifacts
        if (scaledTextSize < fMinDistanceFieldFontSize ||
            scaledTextSize > fMaxDistanceFieldFontSize) {
            return false;
        }

        bool useDFT = props.isUseDeviceIndependentFonts();
#if SK_FORCE_DISTANCE_FIELD_TEXT
        useDFT = true;
#endif

        if (!useDFT && scaledTextSize < kLargeDFFontSize) {
            return false;
        }
    }

    return true;
}

SkScalar scaled_text_size(const SkScalar textSize, const SkMatrix& viewMatrix) {
    SkScalar scaledTextSize = textSize;

    if (viewMatrix.hasPerspective()) {
        // for perspective, we simply force to the medium size
        // TODO: compute a size based on approximate screen area
        scaledTextSize = kMediumDFFontLimit;
    } else {
        SkScalar maxScale = viewMatrix.getMaxScale();
        // if we have non-unity scale, we need to choose our base text size
        // based on the SkPaint's text size multiplied by the max scale factor
        // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
        if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
            scaledTextSize *= maxScale;
        }
    }

    return scaledTextSize;
}

SkFont GrTextContext::Options::getSDFFont(const SkFont& font,
                                          const SkMatrix& viewMatrix,
                                          SkScalar* textRatio) const {
    SkScalar textSize = font.getSize();
    SkScalar scaledTextSize = scaled_text_size(textSize, viewMatrix);

    SkFont dfFont{font};

    if (scaledTextSize <= kSmallDFFontLimit) {
        *textRatio = textSize / kSmallDFFontSize;
        dfFont.setSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        *textRatio = textSize / kMediumDFFontSize;
        dfFont.setSize(SkIntToScalar(kMediumDFFontSize));
#ifdef SK_BUILD_FOR_MAC
    } else if (scaledTextSize <= kLargeDFFontLimit) {
        *textRatio = textSize / kLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kLargeDFFontSize));
    } else {
        *textRatio = textSize / kExtraLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kExtraLargeDFFontSize));
    }
#else
    } else {
        *textRatio = textSize / kLargeDFFontSize;
        dfFont.setSize(SkIntToScalar(kLargeDFFontSize));
    }
#endif

    dfFont.setEdging(SkFont::Edging::kAntiAlias);
    dfFont.setForceAutoHinting(false);
    dfFont.setHinting(SkFontHinting::kNormal);

    // The sub-pixel position will always happen when transforming to the screen.
    dfFont.setSubpixel(false);
    return dfFont;
}

std::pair<SkScalar, SkScalar> GrTextContext::Options::computeSDFMinMaxScale(
        SkScalar textSize, const SkMatrix& viewMatrix) const {

    SkScalar scaledTextSize = scaled_text_size(textSize, viewMatrix);

    // We have three sizes of distance field text, and within each size 'bucket' there is a floor
    // and ceiling.  A scale outside of this range would require regenerating the distance fields
    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = fMinDistanceFieldFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = fMaxDistanceFieldFontSize;
    }

    // Because there can be multiple runs in the blob, we want the overall maxMinScale, and
    // minMaxScale to make regeneration decisions.  Specifically, we want the maximum minimum scale
    // we can tolerate before we'd drop to a lower mip size, and the minimum maximum scale we can
    // tolerate before we'd have to move to a large mip size.  When we actually test these values
    // we look at the delta in scale between the new viewmatrix and the old viewmatrix, and test
    // against these values to decide if we can reuse or not(ie, will a given scale change our mip
    // level)
    SkASSERT(dfMaskScaleFloor <= scaledTextSize && scaledTextSize <= dfMaskScaleCeil);

    return std::make_pair(dfMaskScaleFloor / scaledTextSize, dfMaskScaleCeil / scaledTextSize);
}

SkPaint GrTextContext::InitDistanceFieldPaint(const SkPaint& paint) {
    SkPaint dfPaint{paint};
    dfPaint.setMaskFilter(GrSDFMaskFilter::Make());
    return dfPaint;
}
