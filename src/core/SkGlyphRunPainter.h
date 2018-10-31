/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunPainter_DEFINED
#define SkGlyphRunPainter_DEFINED

#include "SkGlyphRun.h"

#if SK_SUPPORT_GPU
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkGlyphRunListPainter {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListPainter(
            const SkSurfaceProps& props, SkColorType colorType, SkScalerContextFlags flags);

#if SK_SUPPORT_GPU
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
#endif

    using PaintMasks = std::function<void(SkSpan<const SkMask> mask, const SkPaint& paint)>;
    using PaintMasksCreator = std::function<PaintMasks()>;
    struct PathAndPos {
        const SkPath* path;
        SkPoint position;
    };
    using PaintPaths = std::function<
            void(SkSpan<const PathAndPos> pathsAndPositions, SkScalar scale, const SkPaint& paint)>;
    using PaintPathsCreator = std::function<PaintPaths()>;
    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            PaintMasksCreator paintMasksCreator, PaintPathsCreator paintPathsCreator);

    template <typename PerGlyphT, typename PerPathT>
    void drawGlyphRunAsBMPWithPathFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerGlyphT&& perGlyph, PerPathT&& perPath);

    enum NeedsTransform : bool { kTransformDone = false, kDoTransform = true };

    using ARGBFallback =
    std::function<void(const SkPaint& fallbackPaint, // The run paint maybe with a new text size
                       SkSpan<const SkGlyphID> fallbackGlyphIDs, // Colored glyphs
                       SkSpan<const SkPoint> fallbackPositions,  // Positions of above glyphs
                       SkScalar fallbackTextScale,               // Scale factor for glyph
                       const SkMatrix& glyphCacheMatrix,         // Matrix of glyph cache
                       NeedsTransform handleTransformLater)>;    // Positions / glyph transformed

    // Draw glyphs as paths with fallback to scaled ARGB glyphs if color is needed.
    // PerPath - perPath(const SkGlyph&, SkPoint position)
    // FallbackARGB - fallbackARGB(SkSpan<const SkGlyphID>, SkSpan<const SkPoint>)
    // For each glyph that is not ARGB call perPath. If the glyph is ARGB then store the glyphID
    // and the position in fallback vectors. After all the glyphs are processed, pass the
    // fallback glyphIDs and positions to fallbackARGB.
    template <typename PerPath>
    void drawGlyphRunAsPathWithARGBFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& viewMatrix, SkScalar textScale,
            PerPath&& perPath, ARGBFallback&& fallbackARGB);

    template <typename PerSDFT, typename PerPathT>
    void drawGlyphRunAsSDFWithARGBFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& viewMatrix, SkScalar textRatio,
            PerSDFT&& perSDF, PerPathT&& perPath, ARGBFallback&& perFallback);

private:
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix);
    void ensureBitmapBuffers(size_t runSize);

    void processARGBFallback(
            SkScalar maxGlyphDimension, const SkPaint& runPaint, SkPoint origin,
            const SkMatrix& viewMatrix, SkScalar textScale, ARGBFallback argbFallback);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;
    size_t fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;

    // Vectors for tracking ARGB fallback information.
    std::vector<SkGlyphID> fARGBGlyphsIDs;
    std::vector<SkPoint>   fARGBPositions;
};

inline static SkRect rect_to_draw(
        const SkGlyph& glyph, SkPoint origin, SkScalar textScale, bool isDFT) {

    SkScalar dx = SkIntToScalar(glyph.fLeft);
    SkScalar dy = SkIntToScalar(glyph.fTop);
    SkScalar width = SkIntToScalar(glyph.fWidth);
    SkScalar height = SkIntToScalar(glyph.fHeight);

    if (isDFT) {
        dx += SK_DistanceFieldInset;
        dy += SK_DistanceFieldInset;
        width -= 2 * SK_DistanceFieldInset;
        height -= 2 * SK_DistanceFieldInset;
    }

    dx *= textScale;
    dy *= textScale;
    width *= textScale;
    height *= textScale;

    return SkRect::MakeXYWH(origin.x() + dx, origin.y() + dy, width, height);
}

// Beware! The following code will end up holding two glyph caches at the same time, but they
// will not be the same cache (which would cause two separate caches to be created).
template <typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsPathWithARGBFallback(
        SkGlyphCacheInterface* pathCache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& viewMatrix, SkScalar textScale,
        PerPathT&& perPath, ARGBFallback&& argbFallback) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.glyphsIDs()) {
        SkPoint position = *positionCursor++;
        if (SkScalarsAreFinite(position.x(), position.y())) {
            const SkGlyph& glyph = pathCache->getGlyphMetrics(glyphID, {0, 0});
            if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                perPath(glyph, origin + position);
            } else {
                SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
                maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
                fARGBGlyphsIDs.push_back(glyphID);
                fARGBPositions.push_back(position);
            }
        }
    }

    if (!fARGBGlyphsIDs.empty()) {
        this->processARGBFallback(
            maxFallbackDimension, glyphRun.paint(), origin, viewMatrix, textScale,
            std::move(argbFallback));

    }
}

template <typename PerGlyphT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerGlyphT&& perGlyph, PerPathT&& perPath) {

    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    SkVector rounding = cache->rounding();
    mapping.postTranslate(rounding.x(), rounding.y());

    auto runSize = glyphRun.runSize();
    this->ensureBitmapBuffers(runSize);
    mapping.mapPoints(fPositions, glyphRun.positions().data(), runSize);
    const SkPoint* mappedPtCursor = fPositions;
    for (auto glyphID : glyphRun.glyphsIDs()) {
        auto mappedPt = *mappedPtCursor++;
        if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
            const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, mappedPt);
            if (SkGlyphCacheCommon::GlyphTooBigForAtlas(glyph)) {
                SkScalar sx = SkScalarFloorToScalar(mappedPt.fX),
                         sy = SkScalarFloorToScalar(mappedPt.fY);

                SkRect glyphRect =
                        rect_to_draw(glyph, {sx, sy}, SK_Scalar1, false);

                if (!glyphRect.isEmpty()) {
                    perPath(glyph, mappedPt);
                }
            } else {
                perGlyph(glyph, mappedPt);
            }
        }
    }
}

template <typename PerSDFT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsSDFWithARGBFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& viewMatrix, SkScalar textScale,
        PerSDFT&& perSDF, PerPathT&& perPath, ARGBFallback&& argbFallback) {
    fARGBGlyphsIDs.clear();
    fARGBPositions.clear();
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    const SkPoint* positionCursor = glyphRun.positions().data();
    for (auto glyphID : glyphRun.glyphsIDs()) {
        const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, {0, 0});
        SkPoint glyphPos = origin + *positionCursor++;
        if (glyph.fMaskFormat == SkMask::kSDF_Format || glyph.isEmpty()) {
            if (!SkGlyphCacheCommon::GlyphTooBigForAtlas(glyph)) {
                perSDF(glyph, glyphPos);
            } else {
                perPath(glyph, glyphPos);
            }
        } else {
            SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
            maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
            fARGBGlyphsIDs.push_back(glyphID);
            fARGBPositions.push_back(glyphPos);
        }
    }

    if (!fARGBGlyphsIDs.empty()) {
        this->processARGBFallback(
            maxFallbackDimension, glyphRun.paint(), origin, viewMatrix, textScale,
            std::move(argbFallback));
    }
}

#endif  // SkGlyphRunPainter_DEFINED
