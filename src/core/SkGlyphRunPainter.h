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
#include "text/GrTextContext.h"
class GrColorSpaceInfo;
class GrRenderTargetContext;
#endif

class SkStrikeCommon {
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
    SkGlyphRunListPainter(const SkSurfaceProps& props,
                          SkColorType colorType,
                          SkScalerContextFlags flags,
                          SkStrikeCacheInterface* strikeCache);

#if SK_SUPPORT_GPU
    // The following two ctors are used exclusively by the GPU, and will always use the global
    // strike cache.
    SkGlyphRunListPainter(const SkSurfaceProps&, const GrColorSpaceInfo&);
    explicit SkGlyphRunListPainter(const GrRenderTargetContext& renderTargetContext);
#endif

    struct PathAndPos {
        const SkPath* path;
        SkPoint position;
    };

    struct GlyphAndPos {
        const SkGlyph* glyph;
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

    template <typename MasksT, typename PathsT>
    void drawGlyphRunAsBMPWithPathFallback(
            const SkPaint& paint, const SkFont& font,
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& deviceMatrix,
            MasksT&& processMasks, PathsT&& processPaths);

    enum NeedsTransform : bool { kTransformDone = false, kDoTransform = true };

    using ARGBFallback =
    std::function<void(const SkPaint& fallbackPaint, // The run paint maybe with a new text size
                       const SkFont& fallbackFont,
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
    template<typename ProcessPathsT, typename ProcessDeviceT, typename ProcessSourceT>
    void drawGlyphRunAsPathWithARGBFallback(
            const SkPaint& runPaint, const SkFont& runFont,
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& viewMatrix,
            ProcessPathsT&& processPaths,
            ProcessDeviceT&& processDevice, ProcessSourceT&& processSource);

#if SK_SUPPORT_GPU
    template <typename ProcessMasksT, typename ProcessPathsT,
              typename ProcessDeviceT, typename ProcessSourceT>
    void drawGlyphRunAsSDFWithARGBFallback(
            const SkPaint& runPaint, const SkFont& runFont,
            const SkGlyphRun& glyphRun, SkPoint origin, const SkMatrix& viewMatrix,
            const GrTextContext::Options& options,
            ProcessMasksT&& perSDF, ProcessPathsT&& perPath,
            ProcessDeviceT&& processDevice, ProcessSourceT&& processSource);
#endif

    // TODO: Make this the canonical check for Skia.
    static bool ShouldDrawAsPath(const SkPaint& paint, const SkFont& font, const SkMatrix& matrix);

private:
    struct ScopedBuffers {
        ScopedBuffers(SkGlyphRunListPainter* painter, int size);
        ~ScopedBuffers();
        SkGlyphRunListPainter* fPainter;
    };

    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRunList& glyphRunList);

    // TODO: Remove once I can hoist ensureBuffers above the list for loop in all cases.
    ScopedBuffers SK_WARN_UNUSED_RESULT ensureBuffers(const SkGlyphRun& glyphRun);

    template<typename ProcessDeviceT, typename ProcessSourceT>
    void processARGBFallback(SkScalar maxGlyphDimension,
                             const SkPaint& runPaint,
                             const SkFont& runFont,
                             const SkMatrix& viewMatrix,
                             SkScalar textScale,
                             ProcessDeviceT&& processDevice,
                             ProcessSourceT&& processSource);

    // The props as on the actual device.
    const SkSurfaceProps fDeviceProps;
    // The props for when the bitmap device can't draw LCD text.
    const SkSurfaceProps fBitmapFallbackProps;
    const SkColorType fColorType;
    const SkScalerContextFlags fScalerContextFlags;

    SkStrikeCacheInterface* const fStrikeCache;

    int fMaxRunSize{0};
    SkAutoTMalloc<SkPoint> fPositions;
    SkAutoTMalloc<GlyphAndPos> fGlyphPos;

    std::vector<GlyphAndPos> fPaths;

    // Vectors for tracking ARGB fallback information.
    std::vector<SkGlyphID> fARGBGlyphsIDs;
    std::vector<SkPoint>   fARGBPositions;
};

#endif  // SkGlyphRunPainter_DEFINED
