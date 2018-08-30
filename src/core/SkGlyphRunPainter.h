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

    //using PerGlyph = std::function<void(const SkGlyph&, SkPoint)>;
    template <typename PerGlyphT, typename PerPathT>
    void drawGlyphRunAsBMPWithPathFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerGlyphT perGlyph, PerPathT perPath);

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

inline static bool glyph_too_big_for_atlas(const SkGlyph& glyph) {
    return glyph.fWidth > 256 || glyph.fHeight > 256;
}

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

template <typename PerGlyphT, typename PerPathT>
void SkGlyphRunListPainter::drawGlyphRunAsBMPWithPathFallback(
        SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
        SkPoint origin, const SkMatrix& deviceMatrix,
        PerGlyphT perGlyph, PerPathT perPath) {

    auto eachGlyph =
            [perGlyph{std::move(perGlyph)}, perPath{std::move(perPath)}]
                    (const SkGlyph& glyph, SkPoint pt, SkPoint mappedPt) {
                if (glyph_too_big_for_atlas(glyph)) {
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
