/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphRunPainter_DEFINED
#define SkGlyphRunPainter_DEFINED

#include "SkDistanceFieldGen.h"
#include "SkGlyphRun.h"
#include "SkScalerContext.h"
#include "SkSurfaceProps.h"
#include "SkTextBlobPriv.h"

#if SK_SUPPORT_GPU
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkGlyphCacheInterface {
public:
    virtual ~SkGlyphCacheInterface() = default;
    virtual SkVector rounding() const = 0;
    virtual const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) = 0;
    virtual bool hasImage(const SkGlyph& glyph) = 0;
    virtual bool hasPath(const SkGlyph& glyph) = 0;
};

class SkGlyphCacheCommon {
public:
    static SkVector PixelRounding(bool isSubpixel, SkAxisAlignment axisAlignment);

    // This assumes that position has the appropriate rounding term applied.
    static SkIPoint SubpixelLookup(SkAxisAlignment axisAlignment, SkPoint position);

    // An atlas consists of plots, and plots hold glyphs. The minimum a plot can be is 256x256.
    // This means that the maximum size a glyph can be is 256x256.
    static constexpr uint16_t kSkSideTooBigForAtlas = 256;

    static bool GlyphTooBigForAtlas(const SkGlyph& glyph);
};

class SkGlyphRunListPainter {
public:
    // Constructor for SkBitmpapDevice.
    SkGlyphRunListPainter(
            const SkSurfaceProps& props, SkColorType colorType, SkScalerContextFlags flags);

#if SK_SUPPORT_GPU
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
#endif

    struct PathAndPos {
        const SkPath* path;
        SkPoint position;
    };
    class BitmapDevicePainter {
    public:
        virtual ~BitmapDevicePainter() = default;

        virtual void paintPaths(SkSpan<const PathAndPos> pathsAndPositions,
                                SkScalar scale,
                                const SkPaint& paint) const = 0;

        virtual void paintMasks(SkSpan<const SkMask> masks, const SkPaint& paint) const = 0;
    };

    void drawForBitmapDevice(
            const SkGlyphRunList& glyphRunList, const SkMatrix& deviceMatrix,
            const BitmapDevicePainter* bitmapDevice);

    template <typename PerEmptyT, typename PerGlyphT, typename PerPathT>
    void drawGlyphRunAsBMPWithPathFallback(
            SkGlyphCacheInterface* cache, const SkGlyphRun& glyphRun,
            SkPoint origin, const SkMatrix& deviceMatrix,
            PerEmptyT&& perEmpty, PerGlyphT&& perGlyph, PerPathT&& perPath);

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

#endif  // SkGlyphRunPainter_DEFINED
