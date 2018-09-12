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

    using PerMask = std::function<void(const SkMask&, const SkGlyph&, SkPoint)>;
    using PerMaskCreator = std::function<PerMask(const SkPaint&, SkArenaAlloc* alloc)>;
    using PerPath = std::function<void(const SkPath*, const SkGlyph&, SkPoint)>;
    using PerPathCreator = std::function<PerPath(
            const SkPaint&, SkScalar matrixScale, SkArenaAlloc* alloc)>;
    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            PerMaskCreator perMaskCreator, PerPathCreator perPathCreator);
    void drawUsingMasks(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun, SkPoint origin,
            const SkMatrix& deviceMatrix, PerMask perMask);
    void drawUsingPaths(
            const SkGlyphRun& glyphRun, SkPoint origin, SkGlyphCache* cache, PerPath perPath) const;

    template <typename PerGlyphT, typename PerPathT>
    void drawGlyphRunAsBMPWithPathFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerGlyphT perGlyph, PerPathT perPath);

    // Draw glyphs as paths with fallback to scaled ARGB glyphs if color is needed.
    // PerPath - perPath(const SkGlyph&, SkPoint position)
    // FallbackARGB - fallbackARGB(SkSpan<const SkGlyphID>, SkSpan<const SkPoint>)
    // For each glyph that is not ARGB call perPath. If the glyph is ARGB then store the glyphID
    // and the position in fallback vectors. After all the glyphs are processed, pass the
    // fallback glyphIDs and positions to fallbackARGB.
    template <typename PerPath, typename FallbackARGB>
    void drawGlyphRunAsPathWithARGBFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, PerPath perPath, FallbackARGB fallbackARGB);

    template <typename PerSDFT, typename PerPathT, typename PerFallbackT>
    void drawGlyphRunAsSDFWithFallback(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, SkScalar textRatio,
            PerSDFT perSDF, PerPathT perPath, PerFallbackT perFallback);

private:
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkMatrix& matrix);
    bool ensureBitmapBuffers(size_t runSize);


    template <typename EachGlyph>
    void forEachMappedDrawableGlyph(
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
            SkGlyphCacheInterface* cache, EachGlyph eachGlyph);

    void drawGlyphRunAsSubpixelMask(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerMask perMask);

    void drawGlyphRunAsFullpixelMask(
            SkGlyphCache* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerMask perMask);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;
    size_t fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
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

// forEachMappedDrawableGlyph handles positioning for mask type glyph handling for both sub-pixel
// and full pixel positioning.
template <typename EachGlyph>
void SkGlyphRunListPainter::forEachMappedDrawableGlyph(
        const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
        SkGlyphCacheInterface* cache, EachGlyph eachGlyph) {
    SkMatrix mapping = deviceMatrix;
    mapping.preTranslate(origin.x(), origin.y());
    SkVector rounding = cache->rounding();
    mapping.postTranslate(rounding.x(), rounding.y());

    auto runSize = glyphRun.runSize();
    if (this->ensureBitmapBuffers(runSize)) {
        mapping.mapPoints(fPositions, glyphRun.positions().data(), runSize);
        const SkPoint* mappedPtCursor = fPositions;
        const SkPoint* ptCursor = glyphRun.positions().data();
        for (auto glyphID : glyphRun.shuntGlyphsIDs()) {
            auto mappedPt = *mappedPtCursor++;
            auto pt = origin + *ptCursor++;
            if (SkScalarsAreFinite(mappedPt.x(), mappedPt.y())) {
                const SkGlyph& glyph = cache->getGlyphMetrics(glyphID, mappedPt);
                eachGlyph(glyph, pt, mappedPt);
            }
        }
    }
}

template <typename PerPathT, typename FallbackARGB>
void SkGlyphRunListPainter::drawGlyphRunAsPathWithARGBFallback(
        SkGlyphCacheInterface* pathCache, const SkGlyphRun& glyphRun,
        SkPoint origin, PerPathT perPath, FallbackARGB fallbackARGB) {
    std::vector<SkGlyphID> fallbackGlyphIDs;
    std::vector<SkPoint> fallbackPositions;
    SkScalar maxFallbackDimension{-SK_ScalarInfinity};

    auto eachGlyph =
            [pathCache, origin, perPath{std::move(perPath)},
             &fallbackGlyphIDs, &fallbackPositions, &maxFallbackDimension]
            (SkGlyphID glyphID, SkPoint position) {
                if (SkScalarsAreFinite(position.x(), position.y())) {
                    const SkGlyph& glyph = pathCache->getGlyphMetrics(glyphID, {0, 0});
                    if (!glyph.isEmpty()) {
                        if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                            perPath(glyph, origin + position);
                        } else {
                            SkScalar largestDimension = std::max(glyph.fWidth, glyph.fHeight);
                            maxFallbackDimension = std::max(maxFallbackDimension, largestDimension);
                            fallbackGlyphIDs.push_back(glyphID);
                            fallbackPositions.push_back(position);
                        }
                    }
                }
            };

    glyphRun.forEachGlyphAndPosition(eachGlyph);

    if (!fallbackGlyphIDs.empty()) {
        fallbackARGB(SkSpan<const SkGlyphID>{fallbackGlyphIDs},
                     SkSpan<const SkPoint>{fallbackPositions},
                     maxFallbackDimension);
    }
}

template <typename PerGlyphT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerGlyphT perGlyph, PerPathT perPath) {

    auto eachGlyph =
            [perGlyph{std::move(perGlyph)}, perPath{std::move(perPath)}]
                    (const SkGlyph& glyph, SkPoint pt, SkPoint mappedPt) {
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
            };

    this->forEachMappedDrawableGlyph(glyphRun, origin, deviceMatrix, cache, eachGlyph);
}

#endif  // SkGlyphRunPainter_DEFINED
